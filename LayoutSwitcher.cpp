/* -------------------------------------------------------------------------------------------------
 * LayoutSwitcher.h - implementation file for CLayoutSwitcher hotkeys handler class that performs 
 * keyboard layout switch for the active window.
 *
 * Shell Extender (livekeys) utility for Windows Vista x86/x64
 * Copyright (c) 2009 Sergey Rybalkin - rybalkinsp@gmail.com
 ------------------------------------------------------------------------------------------------ */
#include "StdAfx.h"
#include "LayoutSwitcher.h"

CLayoutSwitcher::CLayoutSwitcher ( )
{
}

CLayoutSwitcher::~CLayoutSwitcher ( )
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// This method will be called before handler instance will be used. 
// bHandlerID - application-wide unique identifier of this handler instance
// hwndMainWindow - handler of the main application window
// hInstance - handler of this application instance
// fpnRegisterHotkey - pointer to the function that should be used to register hotkeys
// Returns TRUE if initialized successfully, FALSE otherwise.
BOOL CLayoutSwitcher::Initialize ( BYTE bHandlerID , 
                                   HWND hwndMainWindow , 
                                   HINSTANCE hInstance ,
                                   FPN_REGISTER_HOTKEY fpnRegisterHotkey )
{
    CLiveKeysHandler::Initialize ( bHandlerID , hwndMainWindow , hInstance , fpnRegisterHotkey ) ;

    return TRUE ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// This method will be called before application exit.
VOID CLayoutSwitcher::ShutDown ( FPN_UNREGISTER_HOTKEY fpnUnregisterHotkey )
{
    CLiveKeysHandler::ShutDown ( fpnUnregisterHotkey ) ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Is called from low level keyboard hook routine to notify about keydown event, return TRUE to
// hide event from the system.
BOOL CLayoutSwitcher::NotifyKeyDown ( DWORD vkCode )
{
    m_bShiftPressed = ( VK_RSHIFT == vkCode ) ;

    return FALSE ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Is called from low level keyboard hook routine to notify about keyup event, return TRUE to
// hide event from the system.
BOOL CLayoutSwitcher::NotifyKeyUp ( DWORD vkCode )
{
    if ( m_bShiftPressed && ( VK_RSHIFT == vkCode ) )
        SwitchLayout ( ) ;

    m_bShiftPressed = false ;

    return FALSE ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Does all the work - switches layout in the foreground window.
VOID CLayoutSwitcher::SwitchLayout ( )
{
    HWND hFocusWnd = GetForegroundWindow ( ) ;
    if ( NULL == hFocusWnd )
        return ;

    PostMessage ( hFocusWnd , WM_INPUTLANGCHANGEREQUEST , INPUTLANGCHANGE_FORWARD , HKL_NEXT ) ;

    DWORD dwThreadId = GetWindowThreadProcessId ( hFocusWnd , NULL ) ;
    HKL hCurLayout = GetKeyboardLayout ( dwThreadId ) ;

    if ( 0x04090409 == ( int ) hCurLayout )
        SendMessage ( hFocusWnd , WM_INPUTLANGCHANGE , 0x204 , 0x04190419 ) ;
    else
        SendMessage ( hFocusWnd , WM_INPUTLANGCHANGE , 0 , 0x04090409 ) ;
}