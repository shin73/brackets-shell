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

class cef_themed_menu : public cef_main_window_xp
{
public:
    cef_themed_menu();
    virtual ~cef_themed_menu();

    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    void EnforceOwnerDrawnMenus(bool enforce = true);
    void EnforceMenuBackground();

protected:
    virtual void DoPaintNonClientArea(HDC hdc);
    void DoDrawMenuBar(HDC hdc);
    BOOL HandleMeasureItem(LPMEASUREITEMSTRUCT lpMIS);
    BOOL HandleDrawItem(LPDRAWITEMSTRUCT lpDIS);
    BOOL HandleNcCreate();
    BOOL HandleNcDestroy();

    void ComputeMenuBarRect(RECT& rect);

    HBRUSH mBackgroundBrush;
};
