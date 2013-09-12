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
#include "client_handler.h"
#include "cef_host_window.h"
#include "native_menu_model.h"


// external
extern CefRefPtr<ClientHandler> g_handler;


cef_host_window::cef_host_window(void)
{
}
cef_host_window::~cef_host_window(void)
{
}

HWND cef_host_window::SafeGetCefBrowserHwnd()
{
    if (GetBrowser().get() && GetBrowser()->GetHost().get())
        return GetBrowser()->GetHost()->GetWindowHandle();
    return NULL;
}


BOOL cef_host_window::HandleInitMenuPopup(HMENU hMenuPopup)
{
    int count = ::GetMenuItemCount(hMenuPopup);
    void* menuParent = ::getMenuParent(GetBrowser());
    
    for (int i = 0; i < count; i++) {
        UINT id = GetMenuItemID(hMenuPopup, i);

        bool enabled = NativeMenuModel::getInstance(menuParent).isMenuItemEnabled(id);
        UINT flagEnabled = enabled ? MF_ENABLED | MF_BYCOMMAND : MF_DISABLED | MF_BYCOMMAND;
        EnableMenuItem(hMenuPopup, id,  flagEnabled);

        bool checked = NativeMenuModel::getInstance(menuParent).isMenuItemChecked(id);
        UINT flagChecked = checked ? MF_CHECKED | MF_BYCOMMAND : MF_UNCHECKED | MF_BYCOMMAND;
        CheckMenuItem(hMenuPopup, id, flagChecked);
    }
    return TRUE;
}

BOOL cef_host_window::DoCommand(const CefString& commandString, CefRefPtr<CommandCallback> callback/*=0*/)
{
    if (commandString.size() > 0) 
	{
        g_handler->SendJSCommand(GetBrowser(), commandString, callback);
		return TRUE;
    }
    return FALSE;
}

CefString cef_host_window::GetCommandString(UINT commandId)
{
    return NativeMenuModel::getInstance(::getMenuParent(GetBrowser())).getCommandId(commandId);
}

BOOL cef_host_window::DoCommand(UINT commandId, CefRefPtr<CommandCallback> callback/*=0*/)
{
    return DoCommand(GetCommandString(commandId), callback);
}


/*
BOOL cef_host_window::HandleClose()
{
}
BOOL cef_host_window::HandleCommand(UINT commandId)
{

}
*/

LRESULT cef_host_window::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
/*	case WM_COMMAND:
		if (HandleCommand(LOWORD(wParam)))
			return 0L;
		break;
   	case WM_CLOSE:
		if (HandleClose())
			return 0L;
		break;*/
	case WM_INITMENUPOPUP:
		if (HandleInitMenuPopup((HMENU)wParam))
			return 0L;
		break;
    }

    LRESULT lr = cef_dark_window::WindowProc(message, wParam, lParam);
    return lr;
}
