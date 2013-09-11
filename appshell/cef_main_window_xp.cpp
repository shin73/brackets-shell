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
#include "cef_main_window_xp.h"
#include "resource.h"
#include <minmax.h>
#include <objidl.h>
#include <GdiPlus.h>
#include <Uxtheme.h>
#include <Shlwapi.h>

#pragma comment(lib, "gdiplus")

extern HINSTANCE gInstance;

static ULONG_PTR gdiplusToken = NULL;

static void RECT2Rect(Gdiplus::Rect& dest, const RECT& src) {
    dest.X = src.left;
    dest.Y = src.top;   
    dest.Width = ::RectWidth(src);
    dest.Height = ::RectHeight(src);
}

namespace ResourceImage
{
    Gdiplus::Image* FromResource(LPCWSTR lpResourceName)
    {
        HRSRC hResource = ::FindResource(gInstance, lpResourceName, L"PNG");
        if (!hResource) 
            return NULL;

        HGLOBAL hData = ::LoadResource(gInstance, hResource);
        if (!hData)
            return NULL;

        void* data = ::LockResource(hData);
        ULONG size = ::SizeofResource(gInstance, hResource);

        IStream* pStream = ::SHCreateMemStream((BYTE*)data, (UINT)size);

        if (!pStream) 
            return NULL;

        Gdiplus::Image* image = Gdiplus::Image::FromStream(pStream);
        pStream->Release();
        return image;
    }

}

cef_main_window_xp::cef_main_window_xp() :
    mSysCloseButton(0),
    mSysRestoreButton(0),
    mSysMinimizeButton(0),
    mSysMaximizeButton(0),
    mHoverSysCloseButton(0),
    mHoverSysRestoreButton(0),
    mHoverSysMinimizeButton(0),
    mHoverSysMaximizeButton(0),
    mWindowIcon(0),
    mBackgroundBrush(0),
    mCaptionFont(0)
{
    ::ZeroMemory(&mNcMetrics, sizeof(mNcMetrics));
}

cef_main_window_xp::~cef_main_window_xp()
{

}

void cef_main_window_xp::InitDrawingResources()
{
    mNcMetrics.cbSize = sizeof (mNcMetrics);
    ::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &mNcMetrics, 0);

    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    LoadSysButtonImages();

    ::SetWindowTheme(mWnd, L"", L"");

    if (mBackgroundBrush == NULL) {                            
        mBackgroundBrush = ::CreateSolidBrush(CEF_COLOR_BACKGROUND);
    }
}

void cef_main_window_xp::LoadSysButtonImages()
{

    if (mSysCloseButton == NULL) {
        mSysCloseButton = ResourceImage::FromResource(MAKEINTRESOURCE(IDB_CLOSE_BUTTON));
    }

    if (mSysMaximizeButton == NULL) {
        mSysMaximizeButton = ResourceImage::FromResource(MAKEINTRESOURCE(IDB_MAX_BUTTON));
    }

    if (mSysMinimizeButton == NULL) {
        mSysMinimizeButton = ResourceImage::FromResource(MAKEINTRESOURCE(IDB_MIN_BUTTON));
    }

    if (mSysRestoreButton == NULL) {
        mSysRestoreButton = ResourceImage::FromResource(MAKEINTRESOURCE(IDB_RESTORE_BUTTON));
    }

    if (mHoverSysCloseButton == NULL) {
        mHoverSysCloseButton = ResourceImage::FromResource(MAKEINTRESOURCE(IDB_CLOSE_HOVER_BUTTON));
    }

    if (mHoverSysRestoreButton == NULL) {
        mHoverSysRestoreButton = ResourceImage::FromResource(MAKEINTRESOURCE(IDB_RESTORE_HOVER_BUTTON));
    }

    if (mHoverSysMinimizeButton == NULL) {
        mHoverSysMinimizeButton = ResourceImage::FromResource(MAKEINTRESOURCE(IDB_MIN_HOVER_BUTTON));
    }

    if (mHoverSysMaximizeButton == NULL) {
        mHoverSysMaximizeButton = ResourceImage::FromResource(MAKEINTRESOURCE(IDB_MAX_HOVER_BUTTON));
    }

}

BOOL cef_main_window_xp::HandleNcCreate()
{
    InitDrawingResources();
    return FALSE;
}

BOOL cef_main_window_xp::HandleNcDestroy()
{
	TrackNonClientMouseEvents(false);

    BOOL result = cef_main_window::HandleNcDestroy();

    delete mSysCloseButton;
    delete mSysRestoreButton;
    delete mSysMinimizeButton;
    delete mSysMaximizeButton;

    delete mHoverSysCloseButton;
    delete mHoverSysRestoreButton;
    delete mHoverSysMinimizeButton;
    delete mHoverSysMaximizeButton;

    Gdiplus::GdiplusShutdown(gdiplusToken);

    ::DeleteObject(mBackgroundBrush);
    ::DeleteObject(mCaptionFont);

    return result;
}

void cef_main_window_xp::ComputeWindowIconRect(RECT& rect)
{
    int top = mNcMetrics.iBorderWidth;
    int left = mNcMetrics.iBorderWidth;


    if (IsZoomed()) {
        top = 12;
        left = 12;
    }

    ::SetRectEmpty(&rect);
    rect.top =  top;
    rect.left = left;
    rect.bottom = rect.top + ::GetSystemMetrics(SM_CYSMICON);
    rect.right = rect.left + ::GetSystemMetrics(SM_CXSMICON);
}

void cef_main_window_xp::ComputeWindowCaptionRect(RECT& rect)
{
    RECT wr;
    GetWindowRect(&wr);

    int top = mNcMetrics.iBorderWidth;
    int left = mNcMetrics.iBorderWidth;

    if (IsZoomed()) {
        top = 8;
        left = 8;
    }

    rect.top = top;
    rect.bottom = rect.top + ::GetSystemMetrics (SM_CYCAPTION);

    RECT ir;
    ComputeWindowIconRect(ir);

    RECT mr;
    ComputeMinimizeButtonRect(mr);

    rect.left = ir.right + 1;
    rect.right = mr.left - 1;
}

void cef_main_window_xp::ComputeMinimizeButtonRect(RECT& rect)
{
    ComputeMaximizeButtonRect(rect);

    rect.left -= (mSysMinimizeButton->GetWidth() + 1);
    rect.right = rect.left + mSysMinimizeButton->GetWidth();
    rect.bottom = rect.top + mSysMinimizeButton->GetHeight();
}

void cef_main_window_xp::ComputeMaximizeButtonRect(RECT& rect)
{
    ComputeCloseButtonRect(rect);

    rect.left -= (mSysMaximizeButton->GetWidth() + 1);
    rect.right = rect.left + mSysMaximizeButton->GetWidth();
    rect.bottom = rect.top + mSysMaximizeButton->GetHeight();
}

void cef_main_window_xp::ComputeCloseButtonRect(RECT& rect)
{
    int top = 1;
    int right =  mNcMetrics.iBorderWidth;


    if (IsZoomed()) {
        top =  8;
        right = 8;
    }

    RECT wr;
    GetWindowRect(&wr);

    rect.left = ::RectWidth(wr) - right - mSysCloseButton->GetWidth();
    rect.top = top;
    rect.right = rect.left + mSysCloseButton->GetWidth();
    rect.bottom = rect.top + mSysCloseButton->GetHeight();
}

void cef_main_window_xp::DoDrawFrame(HDC hdc)
{
    RECT rectWindow;
    GetWindowRect(&rectWindow);

    RECT rectFrame;

    ::SetRectEmpty(&rectFrame);

    rectFrame.bottom = ::RectHeight(rectWindow);
    rectFrame.right = ::RectWidth(rectWindow);

    ::FillRect(hdc, &rectFrame, mBackgroundBrush);
}

void cef_main_window_xp::DoDrawSystemMenuIcon(HDC hdc)
{
    if (mWindowIcon == 0) {
        mWindowIcon = reinterpret_cast<HICON>(GetClassLongPtr(GCLP_HICONSM));
        if (mWindowIcon == 0)
            mWindowIcon = reinterpret_cast<HICON>(GetClassLongPtr(GCLP_HICON));
    }

    RECT rectIcon;
    ComputeWindowIconRect(rectIcon);

    ::DrawIconEx(hdc, rectIcon.top, rectIcon.left, mWindowIcon, ::RectWidth(rectIcon), ::RectHeight(rectIcon), 0, NULL, DI_NORMAL);
}

void cef_main_window_xp::DoDrawTitlebarText(HDC hdc)
{
    if (mCaptionFont == 0) {
        mCaptionFont = ::CreateFontIndirect(&mNcMetrics.lfCaptionFont);
    }

    HGDIOBJ hPreviousFont = ::SelectObject(hdc, mCaptionFont);        

    int oldBkMode = ::SetBkMode(hdc, TRANSPARENT);
    COLORREF oldRGB = ::SetTextColor(hdc, CEF_COLOR_NORMALTEXT);

    RECT textRect;
    ComputeWindowCaptionRect(textRect);

    int textLength = GetWindowTextLength() + 1;
    LPWSTR szCaption = new wchar_t [textLength + 1];
    ::ZeroMemory(szCaption, textLength + 1);
    int cchCaption = GetWindowText(szCaption, textLength);

    DrawText(hdc, szCaption, cchCaption, &textRect, DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_CENTER|DT_END_ELLIPSIS|DT_NOPREFIX);

    delete []szCaption;
    ::SetTextColor(hdc, oldRGB);
    ::SetBkMode(hdc, oldBkMode);
    ::SelectObject(hdc, hPreviousFont);
}

void cef_main_window_xp::DoDrawSystemIcons(HDC hdc)
{
    Gdiplus::Image* CloseButton = mSysCloseButton;
    Gdiplus::Image* RestoreButton = mSysRestoreButton;
    Gdiplus::Image* MinimizeButton = mSysMinimizeButton;
    Gdiplus::Image* MaximizeButton = mSysMaximizeButton;

    switch ( mNonClientData.mActiveButton )
 	{
 	case HTCLOSE:				
 		CloseButton = mHoverSysCloseButton;
 		break;
 
 	case HTMAXBUTTON:
 		RestoreButton = mHoverSysRestoreButton;
        MaximizeButton = mHoverSysMaximizeButton;
 		break;
 
 	case HTMINBUTTON:
        MinimizeButton = mHoverSysMinimizeButton;
        break ;
 	}

    RECT rcButton;
    Gdiplus::Rect rect;
    Gdiplus::Graphics grpx(hdc);
   
    ComputeCloseButtonRect(rcButton);
    ::RECT2Rect(rect, rcButton);

    grpx.DrawImage(CloseButton, rect);

    ComputeMaximizeButtonRect(rcButton);
    ::RECT2Rect(rect, rcButton);

    if (IsZoomed()) {
        grpx.DrawImage(RestoreButton, rect);
    } else {
        grpx.DrawImage(MaximizeButton, rect);
    }

    ComputeMinimizeButtonRect(rcButton);
    ::RECT2Rect(rect, rcButton);

    grpx.DrawImage(MinimizeButton, rect);
}


void cef_main_window_xp::InitDeviceContext(HDC hdc)
{
    RECT rectClipClient;
    SetRectEmpty(&rectClipClient);
    ComputeLogicalClientRect(rectClipClient);

    // exclude the client area to reduce flicker
    ::ExcludeClipRect(hdc, rectClipClient.left, rectClipClient.top, rectClipClient.right, rectClipClient.bottom);
}

void cef_main_window_xp::DoDrawMenuBar(HDC hdc)
{
}


void cef_main_window_xp::DoPaintNonClientArea(HDC hdc)
{
    HDC hdcOrig = hdc;
    RECT rectWindow;
    GetWindowRect(&rectWindow);

    int Width = ::RectWidth(rectWindow);
    int Height = ::RectHeight(rectWindow);

    HDC dcMem = ::CreateCompatibleDC(hdc);
    HBITMAP bm = ::CreateCompatibleBitmap(hdc, Width, Height);
    HGDIOBJ bmOld = ::SelectObject(dcMem, bm);

    hdc = dcMem;

    InitDeviceContext(hdc);
    InitDeviceContext(hdcOrig);
    DoDrawFrame(hdc);
    DoDrawSystemMenuIcon(hdc);
    DoDrawTitlebarText(hdc);
    DoDrawSystemIcons(hdc);
    DoDrawMenuBar(hdc);

    ::BitBlt(hdcOrig, 0, 0, Width, Height, dcMem, 0, 0, SRCCOPY);

    ::SelectObject(dcMem, bmOld);
    ::DeleteObject(bm);
    ::DeleteDC(dcMem);
}

void cef_main_window_xp::UpdateNonClientArea()
{
    HDC hdc = GetWindowDC();
    DoPaintNonClientArea(hdc);
    ReleaseDC(hdc);
}

BOOL cef_main_window_xp::HandleNcPaint(HRGN hUpdateRegion)
{
    HDC hdc = GetDCEx(hUpdateRegion, DCX_WINDOW|DCX_INTERSECTRGN|DCX_USESTYLE);
    DoPaintNonClientArea(hdc);
    ReleaseDC(hdc);

    return TRUE;
}

BOOL cef_main_window_xp::HandleSysCommand(UINT uType)
{
    return TRUE;
}

int cef_main_window_xp::HandleNcHitTest(LPPOINT ptHit)
{
    RECT rectWindow;
    GetWindowRect(&rectWindow);

    if (!::PtInRect(&rectWindow, *ptHit)) 
        return HTNOWHERE;
    
    RECT rectClient;
    GetClientRect(&rectClient);
    ClientToScreen(&rectClient);

    if (::PtInRect(&rectClient, *ptHit)) 
        return HTCLIENT;

    RECT rectCaption;
    ComputeWindowCaptionRect(rectCaption);
    NonClientToScreen(&rectCaption);

    if (::PtInRect(&rectCaption, *ptHit)) 
        return HTCAPTION;

    RECT rectCloseButton;
    ComputeCloseButtonRect(rectCloseButton);
    NonClientToScreen(&rectCloseButton);

    if (::PtInRect(&rectCloseButton, *ptHit)) 
        return HTCLOSE;

    RECT rectMaximizeButton;
    ComputeMaximizeButtonRect(rectMaximizeButton);
    NonClientToScreen(&rectMaximizeButton);

    if (::PtInRect(&rectMaximizeButton, *ptHit)) 
        return HTMAXBUTTON;

    RECT rectMinimizeButton;
    ComputeMinimizeButtonRect(rectMinimizeButton);
    NonClientToScreen(&rectMinimizeButton);

    if (::PtInRect(&rectMinimizeButton, *ptHit)) 
        return HTMINBUTTON;

    RECT rectSysIcon;
    ComputeWindowIconRect(rectSysIcon);
    NonClientToScreen(&rectSysIcon);

    if (::PtInRect(&rectSysIcon, *ptHit)) 
        return HTSYSMENU;

	// Left Border
	if ( ptHit->x >= rectWindow.left && ptHit->x <= rectWindow.left + ::GetSystemMetrics (SM_CYFRAME))
 	{
		// it's important that we know if the mouse is on a corner so that
		//	the right mouse cursor is displayed
		if ( ptHit->y <= rectWindow.top + ::GetSystemMetrics (SM_CYFRAME))
 			return HTTOPLEFT;
 
		if ( ptHit->y >= rectWindow.bottom - ::GetSystemMetrics (SM_CYFRAME) )
 			return HTBOTTOMLEFT;
 
 		return HTLEFT;
 	}

	// Right Border
	if ( ptHit->x <= rectWindow.right && ptHit->x >= rectWindow.right - ::GetSystemMetrics (SM_CYFRAME)) 
 	{
		// it's important that we know if the mouse is on a corner so that
		//	the right mouse cursor is displayed
		if ( ptHit->y <= rectWindow.top + ::GetSystemMetrics (SM_CYFRAME))
 			return HTTOPRIGHT;
 
		if ( ptHit->y >= rectWindow.bottom - ::GetSystemMetrics (SM_CYFRAME))
 			return HTBOTTOMRIGHT;
 
 		return HTRIGHT;
 	}

	// Top and Bottom Borders
 	if ( ptHit->y <= rectWindow.top + ::GetSystemMetrics (SM_CYFRAME)) 
 		return HTTOP;
 			
	if ( ptHit->y >= rectWindow.bottom - ::GetSystemMetrics (SM_CYFRAME))
 		return HTBOTTOM;

	return HTMENU;
}

void cef_main_window_xp::UpdateNonClientButtons () 
{
    HDC hdc = GetWindowDC();

 	RECT rectCloseButton ;
 	ComputeCloseButtonRect (rectCloseButton) ;
 
 	RECT rectMaximizeButton ;
 	ComputeMaximizeButtonRect ( rectMaximizeButton ) ;
 
 	RECT rectMinimizeButton ;
 	ComputeMinimizeButtonRect ( rectMinimizeButton ) ;

	RECT rectWindow ;
	ComputeLogicalWindowRect ( rectWindow ) ;
    ::ExcludeClipRect (hdc, rectWindow.left, rectWindow.top, rectWindow.right, rectWindow.bottom);

    RECT rectButtons;
    rectButtons.top = rectCloseButton.top;
    rectButtons.right = rectCloseButton.right;
    rectButtons.bottom = rectCloseButton.bottom;
    rectButtons.left = rectMinimizeButton.left;

    HRGN hrgnUpdate = ::CreateRectRgnIndirect(&rectButtons);

    if (::SelectClipRgn(hdc, hrgnUpdate) != NULLREGION) {
        DoDrawFrame(hdc);           // essentially this will erase the buttons
        DoDrawSystemIcons(hdc);
    }

    ::DeleteObject(hrgnUpdate);

    ReleaseDC(hdc);
}

void cef_main_window_xp::HandleNcMouseLeave() 
{
	switch (mNonClientData.mActiveButton)
	{
		case HTCLOSE:
		case HTMAXBUTTON:
		case HTMINBUTTON:
			mNonClientData.mActiveButton = HTNOWHERE;
			mNonClientData.mButtonOver = true;
			mNonClientData.mButtonDown = false;
			UpdateNonClientButtons ();
			break;
	}

	mNonClientData.Reset();
}

BOOL cef_main_window_xp::HandleNcMouseMove(UINT uHitTest)
{
    if (mNonClientData.mActiveButton != uHitTest) 
    {
        mNonClientData.mActiveButton = uHitTest;

 		mNonClientData.mButtonOver = true;
 		mNonClientData.mButtonDown = false;
	 		
		UpdateNonClientButtons ();
	 
 		switch (uHitTest)
 		{
 		case HTCLOSE:
 		case HTMAXBUTTON:
 		case HTMINBUTTON:
            TrackNonClientMouseEvents();
            return TRUE;
 		}
    }

    return FALSE;
}

BOOL cef_main_window_xp::HandleNcLeftButtonDown(UINT uHitTest)
{
	mNonClientData.mActiveButton = uHitTest;
	mNonClientData.mButtonOver = true;
	mNonClientData.mButtonDown = true;

    UpdateNonClientButtons ();

	switch (uHitTest)
	{
	case HTCLOSE:
	case HTMAXBUTTON:
	case HTMINBUTTON:
        TrackNonClientMouseEvents();
		return TRUE;

    default:
        return FALSE;
	}
}


BOOL cef_main_window_xp::HandleNcLeftButtonUp(UINT uHitTest, LPPOINT point)
{
	mNonClientData.mButtonOver = false;
	mNonClientData.mButtonDown = false;

    UpdateNonClientButtons() ;

	switch (mNonClientData.mActiveButton)
	{
	case HTCLOSE:
		SendMessage (WM_SYSCOMMAND, SC_CLOSE, (LPARAM)POINTTOPOINTS(*point));
		mNonClientData.Reset() ;
        TrackNonClientMouseEvents();
		return TRUE;
	case HTMAXBUTTON:
        if (IsZoomed()) 
			SendMessage (WM_SYSCOMMAND, SC_RESTORE, (LPARAM)POINTTOPOINTS(*point));
        else 
		    SendMessage (WM_SYSCOMMAND, SC_MAXIMIZE, (LPARAM)POINTTOPOINTS(*point));
		mNonClientData.Reset() ;
        TrackNonClientMouseEvents();
		return TRUE;
	case HTMINBUTTON:
		if (IsIconic())
			SendMessage (WM_SYSCOMMAND, SC_RESTORE, (LPARAM)POINTTOPOINTS(*point));
		else
			SendMessage (WM_SYSCOMMAND, SC_MINIMIZE, (LPARAM)POINTTOPOINTS(*point));
		mNonClientData.Reset() ;
        TrackNonClientMouseEvents();
		return TRUE;
	}
    
    mNonClientData.Reset();
    return FALSE;
}


LRESULT cef_main_window_xp::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
    case WM_NCMOUSELEAVE:
        HandleNcMouseLeave();
        break;
    case WM_NCMOUSEMOVE:
        if (HandleNcMouseMove((UINT)wParam))
            return 0L;
        break;

    case WM_NCLBUTTONDOWN:
        if (HandleNcLeftButtonDown((UINT)wParam))
            return 0L;
        break;
    case WM_NCLBUTTONUP:
        {
            POINT pt;
            POINTSTOPOINT(pt, lParam);
            if (HandleNcLeftButtonUp((UINT)wParam, &pt))
                return 0L;
        }
        break;
    case WM_NCCREATE:
        if (HandleNcCreate())
            return 0L;
        break;  
    case WM_NCPAINT:
        if (HandleNcPaint((HRGN)wParam)) 
            return 0L;
        break;
    case WM_NCDESTROY:
        if (HandleNcDestroy())
            return 0L;
        break;

    case WM_NCHITTEST:
        {
            POINT pt;
            POINTSTOPOINT(pt, lParam);
            return HandleNcHitTest(&pt);
        }            
    }

    LRESULT lr = cef_main_window::WindowProc(message, wParam, lParam);
    
    // post default message processing
    switch (message)
    {
    case WM_SYSCOMMAND:
        HandleSysCommand((UINT)(wParam & 0xFFF0));
        break;
    case WM_WINDOWPOSCHANGING:
    case WM_WINDOWPOSCHANGED:
    case WM_MOVE:
    case WM_SIZE:
    case WM_SIZING:
    case WM_EXITSIZEMOVE:
    case WM_NCACTIVATE:
    case WM_ACTIVATE:
    case WM_SETTEXT:
        UpdateNonClientArea();
        break;

    }

    return lr;
}