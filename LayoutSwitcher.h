/* -------------------------------------------------------------------------------------------------
 * LayoutSwitcher.h - header file for CLayoutSwitcher hotkeys handler class that performs keyboard 
 * layout switch for the active window.
 *
 * Shell Extender (livekeys) utility for Windows Vista x86/x64
 * Copyright (c) 2009 Sergey Rybalkin - rybalkinsp@gmail.com
 ------------------------------------------------------------------------------------------------ */
#pragma once
#include "LiveKeysHandler.h"

class CLayoutSwitcher : public CLiveKeysHandler
{
public:
    CLayoutSwitcher ( ) ;
    ~CLayoutSwitcher ( ) ;
    
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

    // This method will be called before application exit.
    VOID ShutDown ( FPN_UNREGISTER_HOTKEY fpnUnregisterHotkey ) ;

    // Is called from low level keyboard hook routine to notify about keydown event, return TRUE to
    // hide event from the system.
    BOOL NotifyKeyDown ( DWORD vkCode ) ;

    // Is called from low level keyboard hook routine to notify about keyup event, return TRUE to
    // hide event from the system.
    BOOL NotifyKeyUp ( DWORD vkCode ) ;

private:

    VOID SetLayout ( DWORD dwLayout ) ; // switches keyboard layout in the foreground window.
    DWORD m_dwPrevKey ; // contains code of the currently pressed key.
    DWORD m_dwLastInputTimestamp ; // contains result of the last GetLastInputInfo call.
};