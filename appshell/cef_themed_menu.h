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
#include "cef_main_window_xp.h"

#define CEF_COLOR_MENU_HILITE_BACKGROUND RGB(247, 247, 247)
#define CEF_COLOR_MENU_HOVER_BACKGROUND RGB(45, 46, 48)
#define CEF_COLOR_MENU_SELECTED_TEXT RGB(30, 30, 30)
#define CEF_COLOR_MENU_DISABLED_TEXT RGB(130, 130, 130)

// This is split out into its own class 
//  so that it can easily be templatized and attached
//  to any cef_window class based object which we 
//  may do at some later point to optimize drawing
//  even more for Vista/7 Glass and 8 themeless
class cef_themed_menu : public cef_main_window_xp
{
public:
    cef_themed_menu();
    virtual ~cef_themed_menu();

    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

protected:
    virtual void DoPaintNonClientArea(HDC hdc);
    virtual void DoDrawMenuBar(HDC hdc);

    BOOL HandleMeasureItem(LPMEASUREITEMSTRUCT lpMIS);
    BOOL HandleDrawItem(LPDRAWITEMSTRUCT lpDIS);
    BOOL HandleNcDestroy();
    BOOL HandleNcCreate();
    
    void ComputeMenuBarRect(RECT& rect);

    void InitDrawingResources();
    void InitMenuFont();
    void EnforceOwnerDrawnMenus();
    void EnforceMenuBackground();

    HFONT mMenuFont;
    HBRUSH mHighlightBrush;
    HBRUSH mHoverBrush;
};
