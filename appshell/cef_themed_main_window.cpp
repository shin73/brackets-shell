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
#include "cef_themed_main_window.h"


cef_themed_main_window::cef_themed_main_window() :
    mShowMenuAccelorators(false)
{

}
cef_themed_main_window::~cef_themed_main_window()
{

}

BOOL cef_themed_main_window::HandleCreate()
{
    BOOL result = cef_main_window_xp::HandleCreate();

    return result;
}


void cef_themed_main_window::DoDrawMenuBar(HDC hdc, LPPOINT lpHitTest/*=NULL*/)
{
    RECT rectBar;

    MENUBARINFO mbi = {0};
    mbi.cbSize = sizeof(mbi);

    if (!GetMenuBarInfo(OBJID_MENU, 0, &mbi))
        return;

    int items = ::GetMenuItemCount(mbi.hMenu);

    ::CopyRect(&rectBar, &mbi.rcBar);
    ScreenToNonClient(&rectBar);

    
    int i;
    wchar_t szMenuString[256] = L"";

    HFONT hMenuFont = ::CreateFontIndirect(&mNcMetrics.lfMenuFont);
    HGDIOBJ oldFont = ::SelectObject(hdc, hMenuFont);            

    HBRUSH hbrHighlight = ::CreateSolidBrush(RGB(247, 247, 247));
    HBRUSH hbrHover = ::CreateSolidBrush(RGB(160, 160, 160));
    RECT rectText;
    ::CopyRect(&rectText, &rectBar);
    
    rectText.top += 1;

    for (i = 0; i < items; i++) {
        ::GetMenuString(mbi.hMenu, i, szMenuString, _countof(szMenuString), MF_BYPOSITION);

        COLORREF rgbMenuText = RGB(197, 197, 197);

        RECT rectTemp;
        SetRectEmpty(&rectTemp);

        // Calc the size of this menu item 
        ::DrawText(hdc, szMenuString, wcslen(szMenuString), &rectTemp, DT_LEFT|DT_SINGLELINE|DT_CALCRECT);

        // Determine the menu item state 
        MENUITEMINFO mmi = {0};
        mmi.cbSize = sizeof (mmi);
        mmi.fMask = MIIM_STATE;
        ::GetMenuItemInfo (mbi.hMenu, i, TRUE, &mmi);

        // Highlighted menu items are selected
        if (mmi.fState & MFS_HILITE) {
            RECT rectButton;
            ::CopyRect(&rectButton, &rectText);
            rectButton.right = rectButton.left + rectTemp.right + 12;

            ::FillRect(hdc, &rectButton, hbrHighlight);

            rgbMenuText = RGB(30, 30, 30);

        } else if (mmi.fState & MFS_GRAYED) {
            // Disabled look
            rgbMenuText = RGB(130, 130, 130);
        } else if (lpHitTest) {
            // Hover state
            RECT rectTest;
            ::CopyRect(&rectTest, &rectText);
            rectTest.left += 6;
            rectTest.right = rectTest.left + rectTemp.right;
            NonClientToScreen(&rectTest);

            if (::PtInRect(&rectTest, *lpHitTest)) {
                RECT rectButton;
                ::CopyRect(&rectButton, &rectText);
                rectButton.right = rectButton.left + rectTemp.right + 12;

                ::FillRect(hdc, &rectButton, hbrHover);

                rgbMenuText = RGB(10, 10, 10);
            }
        }

        // This 6 is a fudge -- the menu looks like this 
        //      [   FILE   ] whereas the area between the brackets 
        //  is what shows us the selection.  There is no metric that 
        //  i can find that represents this correct.
        rectText.left += 6;

        COLORREF oldRGB = ::SetTextColor(hdc, rgbMenuText);
        int oldBkMode   = ::SetBkMode(hdc, TRANSPARENT);
        UINT format = DT_LEFT|DT_SINGLELINE;
       
        if (!mShowMenuAccelorators) 
            format  |= DT_HIDEPREFIX;

        ::DrawText(hdc, szMenuString, wcslen(szMenuString), &rectText, format);
        ::SetTextColor(hdc, oldRGB);
        ::SetBkMode(hdc, oldBkMode);

        // This 8 is a fudge... We start the next one over 2px
        rectText.left += (rectTemp.right + 8);
    }

    //
    ::SelectObject(hdc, oldFont);            
    ::DeleteObject(hbrHighlight);
    ::DeleteObject(hbrHover);
    ::DeleteObject(hMenuFont);
}

void cef_themed_main_window::ComputeMenuBarRect(RECT& rect)
{
    MENUBARINFO mbi = {0};
    mbi.cbSize = sizeof(mbi);
    
    GetMenuBarInfo(OBJID_MENU, 0, &mbi);

    ::CopyRect(&rect, &mbi.rcBar);
    ScreenToNonClient(&rect);
}


void cef_themed_main_window::UpdateMenuBar(LPPOINT lpHitTest/*=NULL*/)
{
    HDC hdc = GetWindowDC();

	RECT rectWindow ;
	ComputeLogicalWindowRect (rectWindow) ;
    ::ExcludeClipRect (hdc, rectWindow.left, rectWindow.top, rectWindow.right, rectWindow.bottom);

    RECT rectMenuBar;
    ComputeMenuBarRect(rectMenuBar);
    HRGN rgnUpdate = ::CreateRectRgnIndirect(&rectMenuBar);

    if (::SelectClipRgn(hdc, rgnUpdate) != NULLREGION) {
        DoDrawFrame(hdc);           
        DoDrawMenuBar(hdc, lpHitTest);
    } 
    ::DeleteObject(rgnUpdate);
    ReleaseDC(hdc);
}

void cef_themed_main_window::EnforceOwnerDrawnMenus()
{
    HMENU hm = GetMenu();
    int items = ::GetMenuItemCount(hm);
    for (int i = 0; i < items; i++) {
        MENUITEMINFO mmi = {0};
        mmi.cbSize = sizeof (mmi);
        mmi.fMask = MIIM_FTYPE;

        ::GetMenuItemInfo(hm, i, TRUE, &mmi);
        mmi.fType |= MFT_OWNERDRAW;
        ::SetMenuItemInfo(hm, i, TRUE, &mmi);
    }
}

void cef_themed_main_window::DoPaintNonClientArea(HDC hdc)
{
    EnforceOwnerDrawnMenus();
    cef_main_window_xp::DoPaintNonClientArea(hdc);
    DoDrawMenuBar(hdc);
}

int cef_themed_main_window::HandleNcHitTest(LPPOINT ptHit)
{
    int hit = cef_main_window_xp::HandleNcHitTest(ptHit);
    if (hit == HTMENU) 
    {
        UpdateMenuBar(ptHit);
        TrackNonClientMouseEvents();
    } else {
        UpdateMenuBar();
    }
    return hit;
}

void cef_themed_main_window::HandleNcMouseLeave() 
{
    cef_main_window_xp::HandleNcMouseLeave();
    UpdateMenuBar();
}


BOOL cef_themed_main_window::HandleNcMouseMove(UINT uHitTest, LPPOINT ptHit)
{
    UpdateMenuBar(ptHit);
    if (uHitTest == HTMENU) 
    {
        TrackNonClientMouseEvents();
        return TRUE;
    } 

    return cef_main_window_xp::HandleNcMouseMove(uHitTest);
}

BOOL cef_themed_main_window::HandleNcLeftButtonDown(UINT uHitTest, LPPOINT ptHit)
{
    UpdateMenuBar(ptHit);

	if (uHitTest == HTMENU) {
        TrackNonClientMouseEvents();
		return FALSE;
	} else {
        return cef_main_window_xp::HandleNcLeftButtonDown(uHitTest);
    }
}

BOOL cef_themed_main_window::HandleNcLeftButtonUp(UINT uHitTest, LPPOINT point)
{
    UpdateMenuBar(point);

	if (uHitTest == HTMENU) {
        TrackNonClientMouseEvents();
        return TRUE;
	} else {
        return cef_main_window_xp::HandleNcLeftButtonUp(uHitTest, point);
    }
    
}

void cef_themed_main_window::ShowMenuAccelerators(bool show/*=true*/)
{
#if 0
    mShowMenuAccelorators = show;
    UpdateMenuBar();
#endif
}

BOOL cef_themed_main_window::HandleSysCommand(UINT uType)
{
    if (uType == SC_KEYMENU) {
        ShowMenuAccelerators(!mShowMenuAccelorators);
        return TRUE;
    } 
    return FALSE;
}


LRESULT cef_themed_main_window::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) 
    {
    case WM_NCMOUSELEAVE:
        UpdateMenuBar();
        break;
    case WM_NCMOUSEMOVE:
        {
            POINT pt;
            POINTSTOPOINT(pt, lParam);
            if (HandleNcMouseMove((UINT)wParam, &pt))
                return 0L;
        }
    
        break;
    case WM_NCHITTEST:
        {
            POINT pt;
            POINTSTOPOINT(pt, lParam);
            return HandleNcHitTest(&pt);
        }
    case WM_NCLBUTTONDOWN:
        {
            POINT pt;
            POINTSTOPOINT(pt, lParam);
            if (HandleNcLeftButtonDown((UINT)wParam, &pt))
                return 0L;
        }
        break;
    case WM_NCRBUTTONDOWN:
    case WM_NCRBUTTONUP:
        {
            if (wParam == HTMENU) 
                return 0L;
        }
        break;

    case WM_NCLBUTTONUP:
        {
            POINT pt;
            POINTSTOPOINT(pt, lParam);
            if (HandleNcLeftButtonUp((UINT)wParam, &pt))
                return 0L;
        }
        break;
    case WM_SYSCOMMAND:
        if (HandleSysCommand((UINT)(wParam & 0xFFF0)))
            return 0L;
        break;
    }

    LRESULT lr = cef_main_window_xp::WindowProc(message, wParam, lParam);

    switch (message) 
    {
    case WM_NCACTIVATE:
    case WM_MENUSELECT:
        UpdateMenuBar();
        break;

    case WM_MOUSEACTIVATE:
        if (lParam != HTMENU) {
            UpdateMenuBar();
        } 
        break;
    case WM_SYSCOMMAND:
        HandleSysCommand((UINT)(wParam & 0xFFF0));
        break;
    }



    return lr;
}
