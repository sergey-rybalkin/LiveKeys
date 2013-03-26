/* -------------------------------------------------------------------------------------------------
 * ShellExtender.cpp - defines entry point for the application.
 *
 * Shell Extender (livekeys) utility for Windows Vista x86/x64
 * Copyright (c) 2009 Sergey Rybalkin - rybalkinsp@gmail.com
 ------------------------------------------------------------------------------------------------ */
#include "stdafx.h"
#include "ShellExtender.h"
#include "LiveKeysHandler.h"

#include "LayoutSwitcher.h"
#include "ShellExecutor.h"
#include "ClipboardHistory.h"
#include "DummyTextGenerator.h"
#include "ZOrderChanger.h"
#include "utils.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Constants definition
#define INSTANCE_MUTEX L"livekeys-8437D236-72F9-433b-9DF7-9A27BED5699E"
#define WINDOWCLASS_NAME L"LIVEKEYS"
#define SHUTDOWN_KEY VK_SCROLL // this key press means that application should be closed

#define NUM_LIVEKEYS_HANDLERS 5 // number of registered live keys handlers

////////////////////////////////////////////////////////////////////////////////////////////////////
// Function prototypes
LRESULT CALLBACK WindowProc ( HWND , UINT , WPARAM , LPARAM ) ;
LRESULT CALLBACK LowLevelHookProc ( int , WPARAM , LPARAM ) ;
BYTE    CALLBACK RegisterHandlerHotkey ( BYTE , UINT , UINT ) ;
VOID    CALLBACK UnregisterHandlerHotkey ( BYTE , BYTE ) ;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Global variables
HINSTANCE      g_hInst = NULL ; // application instance
HWND           g_hWnd  = NULL ; // fake window being created at application startup
HHOOK          g_hHook = NULL ; // low level keyboard hook

// Add new CLiveKeysHandler-derived classes to this array
CLiveKeysHandler* g_pHandlers [ NUM_LIVEKEYS_HANDLERS ] ; 

////////////////////////////////////////////////////////////////////////////////////////////////////
// Application entry point
int APIENTRY _tWinMain ( HINSTANCE hInstance ,
                         HINSTANCE hPrevInstance ,
                         LPTSTR    lpCmdLine ,
                         int       nCmdShow )
{
    UNREFERENCED_PARAMETER ( hPrevInstance ) ;
    UNREFERENCED_PARAMETER ( lpCmdLine ) ;
    UNREFERENCED_PARAMETER ( nCmdShow ) ;

    g_hInst = hInstance ;

    // Ensure that only one copy of our application is running.
    HANDLE mutex = CreateMutex ( NULL , FALSE , INSTANCE_MUTEX ) ;
    DWORD dwRetVal = WaitForSingleObject ( mutex , 0 ) ;
    if ( dwRetVal == WAIT_TIMEOUT )
    {
        ShowCustomError ( IDS_CAPTION_ERROR , IDS_ERROR_ALREADY_RUNNING ) ;
        return 1 ;
    }

    // Create a fake window to listen to events.
    WNDCLASSEX wclx =  { 0 } ;
    wclx.cbSize         = sizeof( wclx ) ;
    wclx.lpfnWndProc    = &WindowProc ;
    wclx.hInstance      = hInstance ;
    wclx.lpszClassName  = WINDOWCLASS_NAME ;
    RegisterClassEx ( &wclx ) ;
    g_hWnd = CreateWindow ( WINDOWCLASS_NAME ,
                            0 ,
                            0 ,
                            0 ,
                            0 ,
                            0 ,
                            0 ,
                            HWND_MESSAGE ,
                            0 ,
                            g_hInst ,
                            0 ) ;

    // Initialize array of handlers. Note to app extenders: register your hotkey handlers here.
    g_pHandlers [ 0 ] = new CLayoutSwitcher   ( ) ;
    g_pHandlers [ 1 ] = new CShellExecutor    ( ) ;
    g_pHandlers [ 2 ] = new CClipboardHistory ( ) ;
    g_pHandlers [ 3 ] = new CDummyTextGenerator ( ) ;
    g_pHandlers [ 4 ] = new CZOrderChanger ( ) ;

    for ( BYTE index = 0 ; index < NUM_LIVEKEYS_HANDLERS ; index++ )
    {
        BOOL bInitialized = g_pHandlers [ index ]->Initialize ( 
            index , g_hWnd , g_hInst , RegisterHandlerHotkey ) ;
        if ( !bInitialized )
        {
            ShowCustomError ( IDS_CAPTION_ERROR , IDS_ERROR_HANDLER_NOT_INITIALIZED ) ;
            delete g_pHandlers [ index ] ;
            g_pHandlers [ index ] = NULL ;
        }
    }

    // Set hook on all keyboard events in order to implement hotkeys functionality.
    g_hHook = SetWindowsHookEx ( WH_KEYBOARD_LL , 
                                 LowLevelHookProc , 
                                 GetModuleHandle ( NULL ) , 
                                 0 ) ;
    if ( NULL == g_hHook )
    {
        ShowCustomError ( IDS_CAPTION_ERROR , IDS_ERROR_HOOK_FAILED ) ;
        return 1 ;
    }

    // And finally start message handling loop.
    MSG msg ;
    while ( GetMessage ( &msg , NULL , 0 , 0 ) )
    {
        TranslateMessage ( &msg ) ;
        DispatchMessage ( &msg ) ;
    }

    // Some cleanup here
    for ( BYTE index = 0 ; index < NUM_LIVEKEYS_HANDLERS ; index++ )
        if ( NULL != g_pHandlers [ index ] )
        {
            g_pHandlers [ index ]->ShutDown ( UnregisterHandlerHotkey ) ;
            delete g_pHandlers [ index ] ;
        }

    UnregisterClass ( WINDOWCLASS_NAME , hInstance ) ;
    DestroyWindow ( g_hWnd ) ;
    UnhookWindowsHookEx ( g_hHook ) ;
 
    return 0 ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Handles all events that we will receive
LRESULT CALLBACK WindowProc ( HWND hWnd , UINT uMsg , WPARAM wParam , LPARAM lParam )
{
    bool isValidEvent ;
    BYTE bHandlerID ;
    BYTE bHotkeyID ;
    CClipboardHistory* pHistory ;

    switch ( uMsg )
    {
        case WM_COMMAND :
            return 0 ;

        case WM_CLOSE :
            PostQuitMessage ( 0 ) ;
            return 0 ;

        case WM_HOTKEY :
            // wParam stores hotkey ID which is actually a word value. low byte is our internal
            // hotkey ID and high byte is handler id - it's index in g_pHandlers array.
            bHandlerID = HIBYTE ( ( WORD ) wParam ) ;
            bHotkeyID  = LOBYTE ( ( WORD ) wParam ) ;

            isValidEvent = ( bHandlerID >= 0 ) && 
                           ( bHandlerID < NUM_LIVEKEYS_HANDLERS ) && 
                           ( NULL != g_pHandlers [ bHandlerID ] ) ;
            if ( isValidEvent )
            {
                g_pHandlers [ bHandlerID ]->HandleHotkey ( bHotkeyID , ( DWORD ) lParam ) ;
                return 0 ;
            }

            return DefWindowProc ( hWnd , uMsg , wParam , lParam ) ;
        case WM_HANDLER :
            // WPARAM - identifier of the hot keys handler to notify 
            // LPARAM - will be passed as a parameter to HandleCommand method
            isValidEvent = ( wParam >= 0 ) && 
                           ( wParam < NUM_LIVEKEYS_HANDLERS ) && 
                           ( NULL != g_pHandlers [ wParam ] ) ;
            if ( isValidEvent )
                g_pHandlers [ wParam ]->HandleCommand ( lParam ) ;

            return 0 ;
        case WM_CHANGECBCHAIN : 
            pHistory = ( CClipboardHistory* ) g_pHandlers [ 2 ] ;

            if ( ( HWND ) wParam == pHistory->GetNextWindowInChain ( ) ) 
                pHistory->SetNextWindowInChain ( ( HWND ) lParam ) ;
            else if ( pHistory->GetNextWindowInChain ( ) != NULL )
                SendMessage ( pHistory->GetNextWindowInChain ( ) , uMsg , wParam , lParam ) ; 

            return 0 ;

        case WM_DRAWCLIPBOARD :
            // It appears that SetClipboardViewer function sends WM_DRAWCLIPBOARD message to the
            // window whose handle is being passed to it, but this call is totally useless for us.
            // Also it cannot be passed to the next window in the chain as we don't have it's handle
            // yet. So just ignore this message for the first time.
            if ( NULL == g_hHook ) // Hook is being set up after initialization so that's a good way
                                   // to check whether this is the frist message
                return 0 ;

            pHistory = ( CClipboardHistory* ) g_pHandlers [ 2 ] ;
            pHistory->NotifyDrawClipboard ( uMsg , wParam , lParam ) ;

            return 0 ;

        default :
            return DefWindowProc ( hWnd , uMsg , wParam , lParam ) ;
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Is called from handlers initialization method to register any required hotkeys. This function 
// should take care of unique hotkey IDs that are being assigned. Returns ID of the registered
// hotkeys or 0 if failed.
BYTE CALLBACK RegisterHandlerHotkey ( BYTE bHandlerID , UINT fsModifiers , UINT vkCode )
{
    static BYTE bNextHotkeyID = 0 ;

    // When receiving the WM_HOTKEY message we will need to find out which handler it belongs to. So
    // in order to simplify this we will make hotkey WORD value' low byte equal to hotkey ID and
    // high byte equal to handler id.
    WORD wHotkey = MAKEWORD ( ++bNextHotkeyID , bHandlerID ) ; 
    
    // Check whether we haven't exceeded maximum amount of hotkeys or handlers
    if ( ( 0xff == bNextHotkeyID ) || ( 0xBFFF <= wHotkey ) )
    {
        ShowCustomError ( IDS_CAPTION_ERROR , IDS_ERROR_HANDLER_NOT_INITIALIZED ) ;
        return 0 ;
    }

    BOOL bRegistered = RegisterHotKey ( g_hWnd , wHotkey , fsModifiers , vkCode ) ;
    if ( bRegistered )
        return bNextHotkeyID ;
    else
        return 0 ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Is called from handlers ShutDown method to unregister any required hotkeys.
VOID CALLBACK UnregisterHandlerHotkey ( BYTE bHandlerID , BYTE bHotkeyID )
{
    // hotkey id WORD value' low byte equal to hotkey ID and high byte equal to handler id.
    WORD wHotkey = MAKEWORD ( bHotkeyID , bHandlerID ) ; 
    
    // Check whether we haven't exceeded maximum amount of hotkeys or handlers
    if ( ( 0xff == bHotkeyID ) || ( 0xBFFF <= wHotkey ) )
        ShowCustomError ( IDS_CAPTION_ERROR , IDS_ERROR_HOTKEY_CLEANUP_FAILED ) ;

    if ( !UnregisterHotKey ( g_hWnd , wHotkey ) )
        ShowWin32Error ( GetLastError ( ) , IDS_CAPTION_ERROR , IDS_ERROR_HOTKEY_CLEANUP_FAILED ) ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// A LowLevelHookProc implementation that captures all supported hotkeys
LRESULT CALLBACK LowLevelHookProc ( int nCode , WPARAM wParam , LPARAM lParam )
{
    // MSDN docs specify that both LL keybd & mouse hook should return in this case.
    if ( nCode != HC_ACTION )  
        return CallNextHookEx ( g_hHook , nCode , wParam , lParam ) ;

    KBDLLHOOKSTRUCT* pKb = ( KBDLLHOOKSTRUCT* ) lParam ;
    if ( pKb->flags & LLKHF_INJECTED ) // ignore injected input
        return CallNextHookEx ( g_hHook , nCode , wParam , lParam ) ;

    // Notify all of our handlers about this keyboard event.
    bool bHandled = false ; // this one will be set to true if we need to hide event from system
    if ( ( wParam == WM_KEYDOWN ) || ( wParam == WM_SYSKEYDOWN ) ) // keydown event
    {
        for ( BYTE index = 0 ; index < NUM_LIVEKEYS_HANDLERS ; index++ )
        {
            if ( NULL == g_pHandlers [ index ] )
                continue;
            if ( g_pHandlers [ index ]->NotifyKeyDown ( pKb->vkCode ) )
                bHandled = true ;
        }
    }
    else // keyup event
    {
        for ( BYTE index = 0 ; index < NUM_LIVEKEYS_HANDLERS ; index++ )
        {
            if ( NULL == g_pHandlers [ index ] )
                continue;
            if ( g_pHandlers [ index ]->NotifyKeyUp ( pKb->vkCode ) )
                bHandled = true ;
        }
    }

    // Pressing the stop key is the fast way to shutdown this application. This check should be
    // the last one in this function because it is rearly used.
    if ( SHUTDOWN_KEY == pKb->vkCode )
    {
        PostMessage ( g_hWnd , WM_CLOSE , 0 , 0 ) ;
        bHandled = true ;
    }
    
    if ( bHandled )
        return 1 ;
    else
        return CallNextHookEx ( g_hHook , nCode , wParam , lParam ) ;
}