/* -------------------------------------------------------------------------------------------------
 * DummyTextGenerator.h - header file for CDummyTextGenerator hotkeys handler class that inserts
 * dummy lorem ipsum text in the active window. 
 *
 * Shell Extender (livekeys) utility for Windows Vista x86/x64
 * Copyright (c) 2009 Sergey Rybalkin - rybalkinsp@gmail.com
 ------------------------------------------------------------------------------------------------ */
#pragma once
#include "StdAfx.h"
#include "LiveKeysHandler.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
class CDummyTextGenerator : public CLiveKeysHandler
{
public:
    CDummyTextGenerator ( ) ;
    ~CDummyTextGenerator( ) ;

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
    HINSTANCE m_hInstance ;
    BYTE m_bShowDummyTextMenuHotkeyID ; 
    HWND m_hwndMainWindow ;
};