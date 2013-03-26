/* -------------------------------------------------------------------------------------------------
 * LiveKeysHandler.cpp - implementation file for CLiveKeysHandler class that provides base API for
 * all classes that performs any actions and are being invoked using keyboard.
 *
 * Shell Extender (livekeys) utility for Windows Vista x86/x64 Copyright (c) 2009 Sergey Rybalkin -
 * rybalkinsp@gmail.com
 ------------------------------------------------------------------------------------------------ */
#include "StdAfx.h"
#include "LiveKeysHandler.h"

CLiveKeysHandler::CLiveKeysHandler ( ) 
{
}

CLiveKeysHandler::~CLiveKeysHandler ( )
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// This method will be called before handler instance will be used. 
// bHandlerID - application-wide unique identifier of this handler instance
// hwndMainWindow - handler of the main application window
// hInstance - handler of this application instance
// fpnRegisterHotkey - pointer to the function that should be used to register hotkeys
// Returns TRUE if initialized successfully, FALSE otherwise.
BOOL CLiveKeysHandler::Initialize ( BYTE bHandlerID , 
                                    HWND hwndMainWindow , 
                                    HINSTANCE hInstance ,
                                    FPN_REGISTER_HOTKEY fpnRegisterHotkey )
{
    m_bHandlerID = bHandlerID ;
    UNREFERENCED_PARAMETER ( hwndMainWindow ) ;
    UNREFERENCED_PARAMETER ( hInstance ) ;
    UNREFERENCED_PARAMETER ( fpnRegisterHotkey ) ;

    return TRUE ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// This method will be called before application exit.
VOID CLiveKeysHandler::ShutDown ( FPN_UNREGISTER_HOTKEY fpnUnregisterHotkey )
{
    UNREFERENCED_PARAMETER ( fpnUnregisterHotkey ) ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Is called from low level keyboard hook routine to notify about keydown event, return TRUE to
// hide event from the system.
BOOL CLiveKeysHandler::NotifyKeyDown ( DWORD vkCode )
{
    UNREFERENCED_PARAMETER ( vkCode ) ;
    return FALSE ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Is called from low level keyboard hook routine to notify about keyup event, return TRUE to
// hide event from the system.
BOOL CLiveKeysHandler::NotifyKeyUp ( DWORD vkCode )
{
    UNREFERENCED_PARAMETER ( vkCode ) ;
    return FALSE ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Is called every time WM_HANDLER window message is received.
VOID CLiveKeysHandler::HandleCommand ( LPARAM lParam )
{
    UNREFERENCED_PARAMETER ( lParam ) ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Is called every time WM_HOTKEY window message is received.
VOID CLiveKeysHandler::HandleHotkey ( BYTE bHotkeyID , DWORD dwHotkey )
{
    UNREFERENCED_PARAMETER ( bHotkeyID ) ;
    UNREFERENCED_PARAMETER ( dwHotkey ) ;
}