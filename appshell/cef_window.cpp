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
#include "cef_window.h"

extern HINSTANCE gInstance;

struct HookData {
    HookData() 
    {
        this->Reset();
    }
    void Reset()
    {
        mOldHook = NULL;
        mWindow = NULL;        
    }

    HHOOK       mOldHook;
    cef_window* mWindow;
} gHookData;




cef_window::cef_window(void) :
    mWnd(NULL),
    mSuperWndProc(NULL)
{
}


cef_window::~cef_window(void)
{
}

static LRESULT CALLBACK _HookProc(int code, WPARAM wParam, LPARAM lParam)
{
    if (code != HCBT_CREATEWND)
        return CallNextHookEx(gHookData.mOldHook, code, wParam, lParam);
    
    LPCREATESTRUCT lpcs = ((LPCBT_CREATEWND)lParam)->lpcs;

    if (lpcs->lpCreateParams == (LPVOID)gHookData.mWindow) 
    {
        HWND hWnd = (HWND)wParam;
        gHookData.mWindow->SubclassWindow(hWnd);
        gHookData.Reset();
    }

    return CallNextHookEx(gHookData.mOldHook, code, wParam, lParam);
}

static void _HookWindowCreate(cef_window* window)
{
    if (gHookData.mOldHook || gHookData.mWindow) 
        return;

    gHookData.mOldHook = ::SetWindowsHookEx(WH_CBT, _HookProc, NULL, ::GetCurrentThreadId());
    gHookData.mWindow = window;
}


static void _UnHookWindowCreate()
{
    gHookData.Reset();
}

static LRESULT CALLBACK _WindowProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  cef_window* window = (cef_window*)::GetProp(hWnd, L"CefClientWindowPtr");
  if (window) 
  {
      return window->WindowProc(message, wParam, lParam);
  } 
  else 
  {
      return DefWindowProc(hWnd, message, wParam, lParam);
  }
}

bool cef_window::SubclassWindow(HWND hWnd) 
{
    if (::GetProp(hWnd, L"CefClientWindowPtr") != NULL) 
    {
        return false;
    }
	mWnd = hWnd;
    mSuperWndProc = (WNDPROC)SetWindowLongPtr(GWLP_WNDPROC, (LONG_PTR)&_WindowProc);
    SetProp(L"CefClientWindowPtr", (HANDLE)this);
    return true;
}

cef_window* cef_window::Create(LPCTSTR szClassname, LPCTSTR szWindowTitle, DWORD dwStyles, int x, int y, int width, int height, cef_window* parent/*=NULL*/, cef_menu* menu/*=NULL*/)
{
    HWND hWndParent = parent ? parent->mWnd : NULL;
    HMENU hMenu = /*menu ? menu->m_hMenu :*/ NULL;

    cef_window* window = new cef_window();

    ::_HookWindowCreate(window);

    HWND hWnd = ::CreateWindow(szClassname, szWindowTitle, 
                               dwStyles, x, y, width, height, hWndParent, hMenu, gInstance, (LPVOID)window);


    ::_UnHookWindowCreate();

    if (hWnd == NULL) 
    {
        delete window;
        return NULL;
    }

    window->mWnd = hWnd;
    return window;
}

LRESULT cef_window::DefaultWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (mSuperWndProc)
    {
        return ::CallWindowProc(mSuperWndProc, mWnd, message, wParam, lParam);
    } 
    else 
    {
        return ::DefWindowProc(mWnd, message, wParam, lParam);
    }

}

BOOL cef_window::HandleNonClientDestroy()
{
	WNDPROC superWndProc = WNDPROC(GetWindowLongPtr(GWLP_WNDPROC));

	RemoveProp(L"CefClientWindowPtr");

	DefaultWindowProc(WM_NCDESTROY, 0, 0);
	
	if ((WNDPROC(GetWindowLongPtr(GWLP_WNDPROC)) == superWndProc) && (mSuperWndProc != NULL))
		SetWindowLongPtr(GWLP_WNDPROC, reinterpret_cast<INT_PTR>(mSuperWndProc));
	
	mSuperWndProc = NULL;
	return TRUE;
}

void cef_window::PostNonClientDestory()
{
    mWnd = NULL;
}

LRESULT cef_window::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
	{
		case WM_NCDESTROY:
			if (HandleNonClientDestroy())
				return 0L;
			break;
	}
	
	LRESULT lr = DefaultWindowProc(message, wParam, lParam);
	
	if (message == WM_NCDESTROY) 
    {
		PostNonClientDestory();
    }

	return lr;
}