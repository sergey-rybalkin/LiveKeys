/* -------------------------------------------------------------------------------------------------
 * ZOrderChanger.h - header file for CZOrderChanger hotkeys handler class that provides shortcuts 
 * for changing Z order of the foreground window.
 *
 * Shell Extender (livekeys) utility for Windows Vista x86/x64
 * Copyright (c) 2012 Sergey Rybalkin - rybalkinsp@gmail.com
 ------------------------------------------------------------------------------------------------ */
#pragma once
#include "StdAfx.h"
#include "LiveKeysHandler.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Globals

////////////////////////////////////////////////////////////////////////////////////////////////////
class CZOrderChanger : public CLiveKeysHandler
{
public:
    CZOrderChanger ( ) ;
    ~CZOrderChanger ( ) ;

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
    BYTE m_bStayOnTopHotkeyID ;
};