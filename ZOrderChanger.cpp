/* -------------------------------------------------------------------------------------------------
 * ZOrderChanger.h - implementation file for CZOrderChanger hotkeys handler class that provides 
 * shortcuts for changing Z order of the foreground window.
 *
 * Shell Extender (livekeys) utility for Windows Vista x86/x64
 * Copyright (c) 2012 Sergey Rybalkin - rybalkinsp@gmail.com
 ------------------------------------------------------------------------------------------------ */
#include "StdAfx.h"
#include "utils.h"
#include "ZOrderChanger.h"


CZOrderChanger::CZOrderChanger ( )
{
    m_bStayOnTopHotkeyID = 0 ;
}


CZOrderChanger::~CZOrderChanger ( )
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
// This method will be called before handler instance will be used. 
// bHandlerID - application-wide unique identifier of this handler instance
// hwndMainWindow - handler of the main application window
// hInstance - handler of this application instance
// fpnRegisterHotkey - pointer to the function that should be used to register hotkeys
// Returns TRUE if initialized successfully, FALSE otherwise.
BOOL CZOrderChanger::Initialize ( BYTE bHandlerID , 
                                  HWND hwndMainWindow , 
                                  HINSTANCE hInstance ,
                                  FPN_REGISTER_HOTKEY fpnRegisterHotkey )
{
    CLiveKeysHandler::Initialize ( bHandlerID , hwndMainWindow , hInstance , fpnRegisterHotkey ) ;

	m_bStayOnTopHotkeyID = fpnRegisterHotkey ( bHandlerID , MOD_CONTROL | MOD_WIN , VkKeyScan ( L'q' ) ) ;

    return m_bStayOnTopHotkeyID > 0 ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Is called every time WM_HOTKEY window message is received. We have only one hotkey - the one that
// displays popup menu with the recent clipboard history items listed. Once user selects something
// from this menu we will paste it into the windows clipboard.
VOID CZOrderChanger::HandleHotkey ( BYTE bHotkeyID , DWORD dwHotkey )
{
    UNREFERENCED_PARAMETER ( dwHotkey ) ;
    UNREFERENCED_PARAMETER ( bHotkeyID ) ;

    HWND hForegroundWindow = GetForegroundWindow ( ) ;
    if ( NULL == hForegroundWindow )
        return ;

    // Check whether window is already topmost
    LONG lExStyles = GetWindowLong ( hForegroundWindow , GWL_EXSTYLE ) ;
    HWND hwndAfter = HWND_TOPMOST ;

    // If it is then remove this flag
    if ( lExStyles & WS_EX_TOPMOST )
        hwndAfter = HWND_NOTOPMOST ;

    SetWindowPos ( hForegroundWindow , hwndAfter , 0 , 0 , 0 , 0 , SWP_NOSIZE | SWP_NOMOVE ) ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// This method will be called before application exit.
VOID CZOrderChanger::ShutDown ( FPN_UNREGISTER_HOTKEY fpnUnregisterHotkey )
{
    CLiveKeysHandler::ShutDown ( fpnUnregisterHotkey ) ;

    fpnUnregisterHotkey ( m_bHandlerID , m_bStayOnTopHotkeyID ) ;
}