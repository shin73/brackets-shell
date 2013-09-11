#pragma once
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
#include "cef_main_window.h"

namespace Gdiplus
{
    class Image;
};

// Color Constants
#define CEF_COLOR_BACKGROUND RGB(60, 63, 65)
#define CEF_COLOR_NORMALTEXT RGB(215, 216, 217)



class cef_main_window_xp : public cef_main_window
{
    class NonClientData
    {
    public:
	    NonClientData() 
	    {
		    Reset() ;
	    }

	    void Reset ( void ) 
	    {
		    mActiveButton = 0 ;
		    mButtonDown = false ;
		    mButtonOver = false ;
	    }

	    UINT mActiveButton;
	    bool mButtonDown ;
	    bool mButtonOver ;
    };

public:
    cef_main_window_xp();
    virtual ~cef_main_window_xp();

    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

protected:
    BOOL HandleNcCreate();
    BOOL HandleNcDestroy();
    BOOL HandleNcPaint(HRGN hUpdateRegion);
    BOOL HandleSysCommand(UINT uType);
    BOOL HandleNcMouseMove(UINT uHitTest);
    BOOL HandleNcLeftButtonUp(UINT uHitTest, LPPOINT point);
    BOOL HandleNcLeftButtonDown(UINT uHitTest);

    int HandleNcHitTest(LPPOINT ptHit);

    void HandleNcMouseLeave();

    void UpdateNonClientArea();
    void UpdateNonClientButtons();

    virtual void InitDeviceContext(HDC hdc);
    virtual void DoPaintNonClientArea(HDC hdc);
    virtual void DoDrawFrame(HDC hdc);
    virtual void DoDrawSystemMenuIcon(HDC hdc);
    virtual void DoDrawTitlebarText(HDC hdc);
    virtual void DoDrawSystemIcons(HDC hdc);
    virtual void DoDrawMenuBar(HDC hdc);

    virtual void ComputeWindowIconRect(RECT& rect);
    virtual void ComputeWindowCaptionRect(RECT& rect);
    virtual void ComputeMinimizeButtonRect(RECT& rect);
    virtual void ComputeMaximizeButtonRect(RECT& rect);
    virtual void ComputeCloseButtonRect(RECT& rect);

    void InitDrawingResources();
    void LoadSysButtonImages();

    Gdiplus::Image* mSysCloseButton;
    Gdiplus::Image* mSysRestoreButton;
    Gdiplus::Image* mSysMinimizeButton;
    Gdiplus::Image* mSysMaximizeButton;
    Gdiplus::Image* mHoverSysCloseButton;
    Gdiplus::Image* mHoverSysRestoreButton;
    Gdiplus::Image* mHoverSysMinimizeButton;
    Gdiplus::Image* mHoverSysMaximizeButton;

    HFONT            mCaptionFont;
    HBRUSH           mBackgroundBrush;
    HICON            mWindowIcon;
    NonClientData    mNonClientData;
    NONCLIENTMETRICS mNcMetrics;
};