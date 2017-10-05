/* -------------------------------------------------------------------------------------------------
 * ClipboardHistory.h - header file for CClipboardHistory hotkeys handler class that stores
 * clipboard history and allows to paste previously used text.
 *
 * Shell Extender (livekeys) utility for Windows Vista x86/x64
 * Copyright (c) 2009 Sergey Rybalkin - rybalkinsp@gmail.com
 ------------------------------------------------------------------------------------------------ */
#pragma once
#include "StdAfx.h"
#include "LiveKeysHandler.h"

 ////////////////////////////////////////////////////////////////////////////////////////////////////
 // Globals
#define MAX_CLIPBOARD_LEN 10000 // Defines maximum clipboard text len that will be handled in bytes
#define MAX_HISTORY_DEPTH 20    // Defines maximum number of items in history
#define MAX_MENU_ITEM_LENGTH 30 // Defines maximum popup menu item text length

struct ClipboardHistoryPoint
{
    WCHAR* lpszContent;
    ClipboardHistoryPoint* pPrevious;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
class CClipboardHistory : public CLiveKeysHandler
{
public:
    CClipboardHistory();
    ~CClipboardHistory();

    // This method will be called before handler instance will be used. 
    // bHandlerID - application-wide unique identifier of this handler instance
    // hwndMainWindow - handler of the main application window
    // hInstance - handler of this application instance
    // fpnRegisterHotkey - pointer to the function that should be used to register hotkeys
    // Returns TRUE if initialized successfully, FALSE otherwise.
    BOOL Initialize(
        BYTE bHandlerID,
        HWND hwndMainWindow,
        HINSTANCE hInstance,
        FPN_REGISTER_HOTKEY fpnRegisterHotkey);

    // Is called every time WM_HOTKEY window message is received.
    VOID HandleHotkey(BYTE bHotkeyID, DWORD dwHotkey);

    // This method will be called before application exit.
    VOID ShutDown(FPN_UNREGISTER_HOTKEY fpnUnregisterHotkey);

    /* As we are using clipboard viewers chain technology here we need to handle some specific
     * window messages here and CLiveKeysHandler API is not enough. Below methods are called
     * directly from main window procedure.
     */

    // Is called when main window receives WM_DRAWCLIPBOARD message
    VOID NotifyClipboardUpdate(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    VOID PushHistoryPoint(WCHAR* lpszClipboardContent, size_t cbLen);
    VOID PasteHistoryPoint(BYTE bIndex);

    HWND m_hwndNextChain; // stores window that is the next in current clipboard viewers chain
    HWND m_hwndMainWindow;
    BYTE m_bShowHistoryHotkeyID;
    ClipboardHistoryPoint* m_pFirst; // Pointer to the root of the linked list - the latest content
                                      // taken from the clipboard
    BYTE m_bCurrentHistoryDepth; // Contains number of elements in pFirst linked list
};