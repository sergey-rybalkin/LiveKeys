/* -------------------------------------------------------------------------------------------------
 * LiveKeysHandler.h - header file for CLiveKeysHandler class that provides base API for all classes 
 * that performs any actions and are being invoked using keyboard events.
 *
 * Shell Extender (livekeys) utility for Windows Vista x86/x64
 * Copyright (c) 2009 Sergey Rybalkin - rybalkinsp@gmail.com
 ------------------------------------------------------------------------------------------------ */
#include "StdAfx.h"
#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////
// Globals

// When this message is sent to the main application window it calls overriden HandleCommand method
// of the hot keys handler. 
// WPARAM - identifier of the hot keys handler to notify 
// LPARAM - will be passed as a parameter to HandleCommand method
#define WM_HANDLER WM_USER+1

// Pointer to the function of this type will be passed to the Initialize method so that handler
// instance will be able to register required hotkeys. Returns ID of the registered hotkeys or 0 if
// failed.
typedef BYTE ( WINAPI *FPN_REGISTER_HOTKEY ) ( BYTE bHandlerID , UINT fsModifiers , UINT vk ) ;

// Pointer to the function of this type will be passed to the ShutDown method so that handler
// instance will be able to Unregister required hotkeys.
typedef VOID ( WINAPI *FPN_UNREGISTER_HOTKEY ) ( BYTE bHandlerID , BYTE bHotkeyID ) ;   

////////////////////////////////////////////////////////////////////////////////////////////////////
// All hotkey hanlder classes should inherit this one
class CLiveKeysHandler
{
public :
    CLiveKeysHandler ( ) ;
    ~CLiveKeysHandler ( ) ;

    // This method will be called before handler instance will be used. 
    // bHandlerID - application-wide unique identifier of this handler instance
    // hwndMainWindow - handler of the main application window
    // hInstance - handler of this application instance
    // fpnRegisterHotkey - pointer to the function that should be used to register hotkeys
    // Returns TRUE if initialized successfully, FALSE otherwise.
    virtual BOOL Initialize ( BYTE bHandlerID , 
                              HWND hwndMainWindow , 
                              HINSTANCE hInstance ,
                              FPN_REGISTER_HOTKEY fpnRegisterHotkey ) ;

    // This method will be called before application exit.
    virtual VOID ShutDown ( FPN_UNREGISTER_HOTKEY fpnUnregisterHotkey ) ;

    // Is called from low level keyboard hook routine to notify about keydown event, return TRUE to
    // hide event from the system.
    virtual BOOL NotifyKeyDown ( DWORD vkCode ) ;

    // Is called from low level keyboard hook routine to notify about keyup event, return TRUE to
    // hide event from the system.
    virtual BOOL NotifyKeyUp ( DWORD vkCode ) ;

    // Is called every time WM_HANDLER window message is received.
    virtual VOID HandleCommand ( LPARAM lParam ) ;

    // Is called every time WM_HOTKEY window message is received.
    virtual VOID HandleHotkey ( BYTE bHotkeyID , DWORD dwHotkey ) ;

protected:
    BYTE m_bHandlerID ;
};