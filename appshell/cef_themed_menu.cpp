/*
 * Copyright (c) 2013 Adobe Systems Incorporated. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#include <stdlib.h>
#include "cef_themed_menu.h"

static const int kMenuItemSpacingCX = 4;
static const int kMenuItemSpacingCY = 4;
static const int kMenuFrameThreshholdCX = 12;

cef_themed_menu::cef_themed_menu() :
    mMenuFont(0),
    mHighlightBrush(0),
    mHoverBrush(0)
{

}

cef_themed_menu::~cef_themed_menu()
{

}

BOOL cef_themed_menu::HandleNcDestroy()
{
    BOOL result = cef_main_window_xp::HandleNcDestroy();

    ::DeleteObject(mMenuFont);
    ::DeleteObject(mHighlightBrush);
    ::DeleteObject(mHoverBrush);

    return result;
}

void cef_themed_menu::InitDrawingResources()
{
    if (mBackgroundBrush == NULL) {                            
        mBackgroundBrush = ::CreateSolidBrush(CEF_COLOR_BACKGROUND);
    }
    if (mHighlightBrush == NULL) {                            
        mHighlightBrush = ::CreateSolidBrush(CEF_COLOR_MENU_HILITE_BACKGROUND);
    }
    if (mHoverBrush == NULL) {                            
        mHoverBrush = ::CreateSolidBrush(CEF_COLOR_MENU_HOVER_BACKGROUND);
    }
}

void cef_themed_menu::InitMenuFont()
{
    if (mMenuFont == NULL) {
        mMenuFont = ::CreateFontIndirect(&mNcMetrics.lfMenuFont);
    }
}


BOOL cef_themed_menu::HandleNcCreate()
{
    BOOL result = cef_main_window_xp::HandleNcCreate();
    InitDrawingResources();
    return result;
}

void cef_themed_menu::EnforceOwnerDrawnMenus()
{
    HMENU hm = GetMenu();
    int items = ::GetMenuItemCount(hm);
    for (int i = 0; i < items; i++) {
        MENUITEMINFO mmi = {0};
        mmi.cbSize = sizeof (mmi);
        mmi.fMask = MIIM_FTYPE;

        ::GetMenuItemInfo(hm, i, TRUE, &mmi);
        if ((mmi.fType & MFT_OWNERDRAW) == 0) {
            mmi.fType |= MFT_OWNERDRAW;
            ::SetMenuItemInfo(hm, i, TRUE, &mmi);
        }
    }
}

void cef_themed_menu::EnforceMenuBackground()
{
    MENUBARINFO mbi = {0};
    mbi.cbSize = sizeof(mbi);
    
    GetMenuBarInfo(OBJID_MENU, 0, &mbi);

    MENUINFO mi = {0};
    mi.cbSize = sizeof(mi);
    mi.fMask = MIM_BACKGROUND;

    ::GetMenuInfo(mbi.hMenu, &mi);
    
    if (mi.hbrBack != mBackgroundBrush) {
        mi.hbrBack = mBackgroundBrush;
        ::SetMenuInfo(mbi.hMenu, &mi);
    }
}

void cef_themed_menu::ComputeMenuBarRect(RECT& rect)
{
    RECT rectClient;
    RECT rectCaption;

    ComputeWindowCaptionRect(rectCaption);
    ComputeLogicalClientRect(rectClient);

    rect.top = rectCaption.bottom + 1;
    rect.bottom = rectClient.top - 1;

    rect.left = rectClient.left;
    rect.right = rectClient.right;
}

void cef_themed_menu::DoPaintNonClientArea(HDC hdc)
{
    EnforceMenuBackground();
    EnforceOwnerDrawnMenus();
    cef_main_window_xp::DoPaintNonClientArea(hdc);
}


void cef_themed_menu::DoDrawMenuBar(HDC hdc)
{
    cef_main_window_xp::DoDrawMenuBar(hdc);

    RECT rectBar;
    ComputeMenuBarRect(rectBar);

    HMENU menu = GetMenu();
    int items = ::GetMenuItemCount(menu);
    
    int i,
        currentTop = rectBar.top + 1,
        currentLeft = rectBar.left;

    for (i = 0; i < items; i++) {
        // Determine the menu item state and ID
        MENUITEMINFO mmi = {0};
        mmi.cbSize = sizeof (mmi);
        mmi.fMask = MIIM_STATE|MIIM_ID;
        ::GetMenuItemInfo (menu, i, TRUE, &mmi);
        
        // Drawitem only works on ID
        MEASUREITEMSTRUCT mis = {0};
        mis.CtlType = ODT_MENU;
        mis.itemID = mmi.wID;

        HandleMeasureItem(&mis);

        RECT itemRect;

        // Compute the rect to draw the item in
        itemRect.top = currentTop + 1;
        itemRect.left = currentLeft + 1 + ::kMenuItemSpacingCX;
        itemRect.right = itemRect.left + mis.itemWidth + ::kMenuItemSpacingCX;
        itemRect.bottom = itemRect.top + mis.itemHeight;

        // check to see if if we need to wrap to a new line
        if (rectBar.left < currentLeft) {
            if (itemRect.right >= (rectBar.right - ::kMenuFrameThreshholdCX)) {
                currentLeft = rectBar.left;
                currentTop = itemRect.bottom - 1;

                itemRect.top = currentTop + 1;
                itemRect.left = currentLeft + 1 + ::kMenuItemSpacingCX;
                itemRect.right = itemRect.left + mis.itemWidth + ::kMenuItemSpacingCX;
                itemRect.bottom = itemRect.top + mis.itemHeight;
            }
        }

        currentLeft = itemRect.right + 1 + ::kMenuItemSpacingCX;

        // Draw the menu item
        DRAWITEMSTRUCT dis = {0};
        dis.CtlType = ODT_MENU;
        dis.itemID = mmi.wID;
        dis.hwndItem = (HWND)menu;
        dis.itemAction = ODA_DRAWENTIRE;
        dis.hDC = hdc;
        ::CopyRect(&dis.rcItem, &itemRect);

        if (mmi.fState & MFS_HILITE) {
            dis.itemState |= ODS_SELECTED;
        } 
        if (mmi.fState & MFS_GRAYED) {
            dis.itemState |= ODS_GRAYED;
        } 

        dis.itemState |= ODS_NOACCEL;

        HandleDrawItem(&dis);
    }

}

BOOL cef_themed_menu::HandleMeasureItem(LPMEASUREITEMSTRUCT lpMIS)
{
    static wchar_t szMenuString[256] = L"";

    if (lpMIS->CtlType == ODT_MENU) {
        HDC dc      = GetWindowDC();
        HMENU menu  = GetMenu();
        int items   = ::GetMenuItemCount(menu);
        
        InitMenuFont();
        
        HGDIOBJ fontOld = ::SelectObject(dc, mMenuFont);            

        ::GetMenuString(menu, lpMIS->itemID, szMenuString, _countof(szMenuString), MF_BYCOMMAND);

        RECT rectTemp;
        SetRectEmpty(&rectTemp);

        // Calc the size of this menu item 
        ::DrawText(dc, szMenuString, ::wcslen(szMenuString), &rectTemp, DT_SINGLELINE|DT_CALCRECT);

        lpMIS->itemHeight = ::RectHeight(rectTemp) + ::kMenuItemSpacingCY;
        lpMIS->itemWidth = ::RectWidth(rectTemp) + ::kMenuItemSpacingCX;

        ::SelectObject(dc, fontOld);            
        ReleaseDC(dc);
        return TRUE;
    }

    return FALSE;
}


BOOL cef_themed_menu::HandleDrawItem(LPDRAWITEMSTRUCT lpDIS)
{
    static wchar_t szMenuString[256] = L"";

    if (lpDIS->CtlType == ODT_MENU) {
        int items = ::GetMenuItemCount((HMENU)lpDIS->hwndItem);

        InitMenuFont();

        HGDIOBJ fontOld = ::SelectObject(lpDIS->hDC, mMenuFont);            
        COLORREF rgbMenuText =  CEF_COLOR_NORMALTEXT;

        ::GetMenuString((HMENU)lpDIS->hwndItem, lpDIS->itemID, szMenuString, _countof(szMenuString), MF_BYCOMMAND);
        
        if (lpDIS->itemState & ODS_SELECTED) {
            ::FillRect(lpDIS->hDC, &lpDIS->rcItem, mHighlightBrush);
            rgbMenuText = CEF_COLOR_MENU_SELECTED_TEXT;
        } else if (lpDIS->itemState & ODS_HOTLIGHT) {
            ::FillRect(lpDIS->hDC, &lpDIS->rcItem, mHoverBrush);
        } else {
            ::FillRect(lpDIS->hDC, &lpDIS->rcItem, mBackgroundBrush);
        }
        
        if (lpDIS->itemState & ODS_GRAYED) {
            rgbMenuText = CEF_COLOR_MENU_DISABLED_TEXT;
        }

        COLORREF oldRGB = ::SetTextColor(lpDIS->hDC, rgbMenuText);
        
        UINT format = DT_CENTER|DT_SINGLELINE;
       
        if (lpDIS->itemState & ODS_NOACCEL) {
            format  |= DT_HIDEPREFIX;
        }

        int oldBkMode   = ::SetBkMode(lpDIS->hDC, TRANSPARENT);

        ::DrawText(lpDIS->hDC, szMenuString, ::wcslen(szMenuString), &lpDIS->rcItem, format);

        ::SelectObject(lpDIS->hDC, fontOld);
        ::SetBkMode(lpDIS->hDC, oldBkMode);
        ::SetTextColor(lpDIS->hDC, oldRGB);

        return TRUE;
    }

    return FALSE;    

}


LRESULT cef_themed_menu::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) 
    {
    case WM_NCCREATE:
        if (HandleNcCreate())
            return 0L;
        break;
    case WM_NCDESTROY:
        if (HandleNcDestroy())
            return 0L;
        break;

    case WM_MEASUREITEM:
        if (HandleMeasureItem((LPMEASUREITEMSTRUCT)lParam))
            return TRUE;
        break;
    case WM_DRAWITEM:
        if (HandleDrawItem((LPDRAWITEMSTRUCT)lParam))
            return TRUE;
        break;
    }
    return cef_main_window_xp::WindowProc(message, wParam, lParam);;
}