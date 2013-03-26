/* -------------------------------------------------------------------------------------------------
 * ExtraClipboard.h - header file for CExtraClipboard hotkeys handler class that adds additional 
 * clipboards to the system.
 *
 * Shell Extender (livekeys) utility for Windows Vista x86/x64
 * Copyright (c) 2009 Sergey Rybalkin - rybalkinsp@gmail.com
 ------------------------------------------------------------------------------------------------ */
#pragma once
#include "StdAfx.h"
#include "LiveKeysHandler.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Globals and constants
#define NUM_CLIPBOARDS 9 // Number of additional keyboards to be added, should be between 0 and 9

////////////////////////////////////////////////////////////////////////////////////////////////////
class CExtraClipboard : public CLiveKeysHandler
{
public:
    CExtraClipboard ( ) ;
    ~CExtraClipboard ( ) ;

    // This method will be called before handler instance will be used. 
    // bHandlerID - application-wide unique identifier of this handler instance
    // hwndMainWindow - handler of the main application window
    // hInstance - handler of this application instance
    // fpnRegisterHotkey - pointer to the function that should be used to register hotkeys
    // Returns TRUE if initialized successfully, FALSE otherwise.
    BOOL Initialize ( BYTE bHandlerID , 
                      HWND hwndMainWindow , 
                      HINSTANCE hInstance ,
                      FPN_REGISTER_HOTKEY fpnRegisterHotkey ) ;

    // Is called every time WM_HOTKEY window message is received.
    VOID HandleHotkey ( BYTE bHotkeyID , DWORD dwHotkey ) ;

    // This method will be called before application exit.
    VOID ShutDown ( FPN_UNREGISTER_HOTKEY fpnUnregisterHotkey ) ;

private:
    VOID CopyToExtraClipboard ( int iClipboardIndex ) ;
    VOID PateFromExtraClipboard ( int iClipboardIndex ) ;
    VOID SendCopyCmd ( ) ;
    VOID SendPasteCmd ( ) ;

    // This field will store pointers to the text buffers allocated for extra clipboards content.
    // Buffers for clipboards content should be allocated from the heap because they will be used in
    // non-C++ threads.
    WCHAR* m_pszClipboardsContent [ NUM_CLIPBOARDS ] ;

    BYTE m_pbCopyHotkeys  [ NUM_CLIPBOARDS ] ;
    BYTE m_pbPasteHotkeys [ NUM_CLIPBOARDS ] ;
};
