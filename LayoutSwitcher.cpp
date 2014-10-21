/* -------------------------------------------------------------------------------------------------
 * LayoutSwitcher.h - implementation file for CLayoutSwitcher hotkeys handler class that performs 
 * keyboard layout switch for the active window.
 *
 * Shell Extender (livekeys) utility for Windows Vista x86/x64
 * Copyright (c) 2009 Sergey Rybalkin - rybalkinsp@gmail.com
 ------------------------------------------------------------------------------------------------ */
#include "StdAfx.h"
#include "LayoutSwitcher.h"

#define RUSSIAN_LAYOUT 0x04190419
#define ENGLISH_LAYOUT 0x04090409

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
    if ( m_dwPrevKey == vkCode )
        return FALSE ;

    if ( vkCode == VK_RSHIFT || vkCode == VK_LSHIFT )
    {
        LASTINPUTINFO info ; 
        info.cbSize = sizeof ( info ) ;
        GetLastInputInfo ( &info ) ;
        m_dwLastInputTimestamp = info.dwTime ;

        m_dwPrevKey = vkCode ;
    }

    return FALSE ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Is called from low level keyboard hook routine to notify about keyup event, return TRUE to
// hide event from the system.
BOOL CLayoutSwitcher::NotifyKeyUp ( DWORD vkCode )
{
    if ( ( m_dwPrevKey & vkCode ) == VK_RSHIFT )
        SetLayout ( ENGLISH_LAYOUT ) ;
    else if ( ( m_dwPrevKey & vkCode ) == VK_LSHIFT )
        SetLayout ( RUSSIAN_LAYOUT ) ;
    
    m_dwPrevKey = 0 ;

    return FALSE ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Does all the work - switches layout in the foreground window.
VOID CLayoutSwitcher::SetLayout ( DWORD dwLayout )
{
    // Check whether hotkey has expired
    DWORD tickCount = GetTickCount();
    if ( tickCount - m_dwLastInputTimestamp > 200 )
        return ;

    OutputDebugString ( L"Pingback from hotkey" ) ;

    HWND hFocusWnd = GetForegroundWindow ( ) ;
    if ( NULL == hFocusWnd )
        return ;

    DWORD dwThreadId = GetWindowThreadProcessId ( hFocusWnd , NULL ) ;
    HKL hCurLayout = GetKeyboardLayout ( dwThreadId ) ;

    // If the specified layout is already activated then we don't need to do anything.
    if ( dwLayout == ( DWORD ) hCurLayout )
        return ;

    // Emulate Win + Space keyboard shortcut that switches languages regardless of the system settings
    INPUT inputs [ 4 ] = { 0 } ;

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_CONTROL;

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_SHIFT;

    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = VK_SHIFT;
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_CONTROL;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput ( 4 , inputs , sizeof ( INPUT ) ) ;
}