/*************************************************************************
 *
 * ADOBE CONFIDENTIAL
 * ___________________
 *
 *  Copyright 2013 Adobe Systems Incorporated
 *  All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Adobe Systems Incorporated and its suppliers,
 * if any.  The intellectual and technical concepts contained
 * herein are proprietary to Adobe Systems Incorporated and its
 * suppliers and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Adobe Systems Incorporated.
 **************************************************************************/
#include <stdlib.h>
#include "cef_themed_menu.h"

cef_themed_menu::cef_themed_menu() :
    mBackgroundBrush(NULL)
{

}
cef_themed_menu::~cef_themed_menu()
{

}

BOOL cef_themed_menu::HandleNcCreate()
{
    BOOL result = cef_main_window_xp::HandleNcCreate();


    return result;
}

BOOL cef_themed_menu::HandleNcDestroy()
{
    ::DeleteObject(mBackgroundBrush);
    mBackgroundBrush = NULL;
    
    return cef_main_window_xp::HandleNcDestroy();
}

void cef_themed_menu::EnforceOwnerDrawnMenus(bool enforce/*=true*/)
{
    HMENU hm = GetMenu();
    int items = ::GetMenuItemCount(hm);
    for (int i = 0; i < items; i++) {
        MENUITEMINFO mmi = {0};
        mmi.cbSize = sizeof (mmi);
        mmi.fMask = MIIM_FTYPE;

        ::GetMenuItemInfo(hm, i, TRUE, &mmi);
        if (enforce)
            mmi.fType |= MFT_OWNERDRAW;
        else 
            mmi.fType &= ~MFT_OWNERDRAW;
        ::SetMenuItemInfo(hm, i, TRUE, &mmi);
    }
}


void cef_themed_menu::EnforceMenuBackground()
{
    if (mBackgroundBrush == NULL) {
        mBackgroundBrush = ::CreateSolidBrush(RGB(0, 0, 0));
    }

    MENUBARINFO mbi = {0};
    mbi.cbSize = sizeof(mbi);
    
    GetMenuBarInfo(OBJID_MENU, 0, &mbi);

    MENUINFO mi = {0};
    mi.cbSize = sizeof(mi);
    mi.fMask = MIM_BACKGROUND;

    ::GetMenuInfo(mbi.hMenu, &mi);

    mi.hbrBack = mBackgroundBrush;

    ::SetMenuInfo(mbi.hMenu, &mi);
}

void cef_themed_menu::ComputeMenuBarRect(RECT& rect)
{
    RECT rectClient;
    RECT rectCaption;

    ComputeWindowCaptionRect(rectCaption);
    ComputeLogicalClientRect(rectClient);

    rect.top = rectCaption.bottom + 2;
    rect.bottom = rectClient.top;

    rect.left = rectClient.left;
    rect.right = rectClient.right;
}

void cef_themed_menu::DoPaintNonClientArea(HDC hdc)
{
    EnforceOwnerDrawnMenus(true);
    cef_main_window_xp::DoPaintNonClientArea(hdc);
    DoDrawMenuBar(hdc);
}


void cef_themed_menu::DoDrawMenuBar(HDC hdc)
{
    RECT rectBar;
    ComputeMenuBarRect(rectBar);

    HMENU menu = GetMenu();
    int items = ::GetMenuItemCount(menu);
    
    int i,
        currentTop = rectBar.top,
        currentLeft = rectBar.left;

    wchar_t szMenuString[256] = L"";


    for (i = 0; i < items; i++) {

        // Determine the menu item state 
        MENUITEMINFO mmi = {0};
        mmi.cbSize = sizeof (mmi);
        mmi.fMask = MIIM_STATE|MIIM_ID;
        ::GetMenuItemInfo (menu, i, TRUE, &mmi);

        
        MEASUREITEMSTRUCT mis = {0};
        mis.CtlType = ODT_MENU;
        mis.itemID = mmi.wID;

        HandleMeasureItem(&mis);

        RECT itemRect;

        itemRect.top = currentTop + 1;
        itemRect.left = currentLeft + 5;
        itemRect.right = itemRect.left + mis.itemWidth + 4;
        itemRect.bottom = itemRect.top + mis.itemHeight;

        if (rectBar.left < currentLeft) {
            if (itemRect.right > rectBar.right) {
                currentLeft = rectBar.left;
                currentTop = itemRect.bottom + 4;

                itemRect.top = currentTop;
                itemRect.left = currentLeft;
                itemRect.right = itemRect.left + mis.itemWidth;
                itemRect.bottom = itemRect.top + mis.itemHeight;
            }
        }

        currentLeft = itemRect.right + 5;

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

        int items       = ::GetMenuItemCount(menu);
        HFONT fontMenu  = ::CreateFontIndirect(&mNcMetrics.lfMenuFont);
        HGDIOBJ fontOld = ::SelectObject(dc, fontMenu);            

        ::GetMenuString(menu, lpMIS->itemID, szMenuString, _countof(szMenuString), MF_BYCOMMAND);

        RECT rectTemp;
        SetRectEmpty(&rectTemp);

        // Calc the size of this menu item 
        ::DrawText(dc, szMenuString, wcslen(szMenuString), &rectTemp, DT_LEFT|DT_SINGLELINE|DT_CALCRECT);

        lpMIS->itemHeight = ::RectHeight(rectTemp) + 4;
        lpMIS->itemWidth = ::RectWidth(rectTemp) + 4 ;

        ::SelectObject(dc, fontOld);            
        ::DeleteObject(fontMenu);

        ReleaseDC(dc);
        return TRUE;
    }

    return FALSE;
}


BOOL cef_themed_menu::HandleDrawItem(LPDRAWITEMSTRUCT lpDIS)
{
    static wchar_t szMenuString[256] = L"";

    if (lpDIS->CtlType == ODT_MENU) {
        int items       =       ::GetMenuItemCount((HMENU)lpDIS->hwndItem);
        HFONT fontMenu  =       ::CreateFontIndirect(&mNcMetrics.lfMenuFont);
        HGDIOBJ fontOld =       ::SelectObject(lpDIS->hDC, fontMenu);            
        HBRUSH hbrHighlight =   ::CreateSolidBrush(RGB(247, 247, 247));
        HBRUSH hbrHover =       ::CreateSolidBrush(RGB(0, 0, 0));
        HBRUSH hbrBackground =  ::CreateSolidBrush(RGB(60, 63, 65));
        COLORREF rgbMenuText =  RGB(255, 255, 255);

        ::GetMenuString((HMENU)lpDIS->hwndItem, lpDIS->itemID, szMenuString, _countof(szMenuString), MF_BYCOMMAND);
        
        if (lpDIS->itemState & ODS_SELECTED) {
            ::FillRect(lpDIS->hDC, &lpDIS->rcItem, hbrHighlight);
            rgbMenuText = RGB(30, 30, 30);
        } else if (lpDIS->itemState & ODS_HOTLIGHT) {
            ::FillRect(lpDIS->hDC, &lpDIS->rcItem, hbrHover);
            rgbMenuText = RGB(255, 255, 255);
        } else {
            ::FillRect(lpDIS->hDC, &lpDIS->rcItem, hbrBackground);
            rgbMenuText = RGB(255, 255, 255);
        }
        
        if (lpDIS->itemState & ODS_GRAYED)
            rgbMenuText = RGB(130, 130, 130);

        COLORREF oldRGB = ::SetTextColor(lpDIS->hDC, rgbMenuText);
        
        UINT format = DT_CENTER|DT_SINGLELINE;
       
        if (lpDIS->itemState & ODS_NOACCEL) 
            format  |= DT_HIDEPREFIX;

        int oldBkMode   = ::SetBkMode(lpDIS->hDC, TRANSPARENT);

        ::DrawText(lpDIS->hDC, szMenuString, ::wcslen(szMenuString), &lpDIS->rcItem, format);

        ::SelectObject(lpDIS->hDC, fontOld);
        ::SetBkMode(lpDIS->hDC, oldBkMode);
        ::SetTextColor(lpDIS->hDC, oldRGB);

        ::DeleteObject(fontMenu);
        ::DeleteObject(hbrHighlight);
        ::DeleteObject(hbrHover);
        ::DeleteObject(hbrBackground);

        return TRUE;
    }

    return FALSE;    

}


LRESULT cef_themed_menu::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) 
    {
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