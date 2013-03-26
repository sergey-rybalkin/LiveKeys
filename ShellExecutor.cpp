/* -------------------------------------------------------------------------------------------------
 * ShellExecutor.h - implementation file for CShellExecutor hotkeys handler class that starts
 * applications or open documents that have hotkeys assigned to them.
 *
 * Shell Extender (livekeys) utility for Windows Vista x86/x64 Copyright (c) 2009 Sergey Rybalkin -
 * rybalkinsp@gmail.com
 ------------------------------------------------------------------------------------------------ */
#include "StdAfx.h"
#include "ShellExecutor.h"
#include "utils.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Constants definition
#define MAX_KEY_LENGTH 255 // maximum supported length of the registry key full name

// Important: in order this class to work qlaunch.exe should be in PATH
TCHAR g_szQLaunchExe     [ ] = TEXT ( "qlaunch.exe" ) ;
TCHAR g_szQLaunchCmd     [ ] = TEXT ( "-o %s" ) ;
TCHAR g_szExecutorRegKey [ ] = TEXT ( "SOFTWARE\\LiveKeys\\Executor" ) ;
TCHAR g_szAsyncKeyRegVal [ ] = TEXT ( "AsyncKeys" ) ;
TCHAR g_szMainKeyRegVal  [ ] = TEXT ( "MainKey" ) ;

CShellExecutor::CShellExecutor ( )
{
    
}

CShellExecutor::~CShellExecutor ( )
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Reads all quick launch items from registry and registers their hotkeys in the system.
BOOL CShellExecutor::Initialize ( BYTE bHandlerID , 
                                  HWND hwndMainWindow , 
                                  HINSTANCE hInstance ,
                                  FPN_REGISTER_HOTKEY fpnRegisterHotkey )
{
    CLiveKeysHandler::Initialize ( bHandlerID , hwndMainWindow , hInstance , fpnRegisterHotkey ) ;

    HKEY hExecutorKey ;

    // Open registry key that stores information about hotkey combinations
	LONG lRetVal = RegOpenKeyEx ( HKEY_CURRENT_USER , 
                                  g_szExecutorRegKey , 
                                  0 , 
                                  KEY_READ , 
                                  &hExecutorKey ) ;
    if ( ERROR_SUCCESS != lRetVal )
    {
        DbgPrintWin32Error ( lRetVal , IDS_ERROR_NO_EXECUTOR_KEY ) ;
        return FALSE ;
    }

    if ( !AllocateMemoryForHotkeys ( hExecutorKey ) )
    {
        DbgPrintWin32Error ( GetLastError ( ) , 0 ) ;
        return FALSE ;
    }
    
    // Read all registered hotkeys by enumerating subkeys of hExecutorKey
    for ( DWORD dwIndex = 0 ; dwIndex < m_cRegisteredHotkeys ; dwIndex++ )
    {
        m_pHotkeys [ dwIndex ].lpszItemName = new TCHAR [ MAX_KEY_LENGTH ] ;

        lRetVal = RegEnumKey ( hExecutorKey ,
                               dwIndex , 
                               m_pHotkeys [ dwIndex ].lpszItemName ,
                               MAX_KEY_LENGTH ) ;
        if ( ERROR_SUCCESS != lRetVal )
        {
            DbgPrintWin32Error ( lRetVal , IDS_ERROR_CANNOT_ENUM_EXECUTOR_KEY ) ;
            RegCloseKey ( hExecutorKey ) ;
            return FALSE ;
        }

        BYTE bHotkeyID = RegisterHotkey ( hExecutorKey , 
                                          m_pHotkeys [ dwIndex ].lpszItemName , 
                                          fpnRegisterHotkey ) ;
        if ( !bHotkeyID )
        {
            DbgPrintWin32Error ( GetLastError ( ) , IDS_ERROR_CANNOT_ENUM_EXECUTOR_KEY ) ;
            RegCloseKey ( hExecutorKey ) ;
            return FALSE ;
        }
        else
            m_pHotkeys [ dwIndex ].bHotkeyID = bHotkeyID ;
    }

    return ( ERROR_SUCCESS == RegCloseKey ( hExecutorKey ) ) ? TRUE : FALSE ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Is called every time WM_HOTKEY window message is received.
VOID CShellExecutor::HandleHotkey ( BYTE bHotkeyID , DWORD dwHotkey )
{
    UNREFERENCED_PARAMETER ( dwHotkey ) ;

    // Try to find id of the specified hotkey in our internal collection
    HOTKEY_INFO* pInfo = NULL ;
    for ( DWORD index = 0 ; index < m_cRegisteredHotkeys ; index++ )
    {
        if ( m_pHotkeys [ index ].bHotkeyID == bHotkeyID )
        {
            pInfo = &m_pHotkeys [ index ] ;
            break ;
        }
    }

    if ( NULL == pInfo )
        return ;

    // If found - run QLaunch.exe in order to open required file/application.
    TCHAR szParameters [ MAX_PATH ] ;
    HRESULT hr = StringCchPrintf ( szParameters , 
                                   MAX_PATH , 
                                   g_szQLaunchCmd , 
                                   pInfo->lpszItemName ) ;
    if ( FAILED ( hr ) )
    {
        DbgPrintWin32Error ( hr , 0 ) ;
        return ;
    }

    ShellExecute ( NULL , TEXT ( "open" ) , g_szQLaunchExe , szParameters , NULL , SW_SHOWNORMAL ) ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// This method will be called before application exit.
VOID CShellExecutor::ShutDown ( FPN_UNREGISTER_HOTKEY fpnUnregisterHotkey )
{
    for ( DWORD index = 0 ; index < m_cRegisteredHotkeys ; index++ )
    {
        HOTKEY_INFO* pInfo = &m_pHotkeys [ index ] ;
        fpnUnregisterHotkey ( m_bHandlerID , pInfo->bHotkeyID ) ;
        delete[] pInfo->lpszItemName ;
    }

    delete[] m_pHotkeys ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Gets count of quick launch items in the registry and allocates required buffers for them
BOOL CShellExecutor::AllocateMemoryForHotkeys ( HKEY hExecutorKey )
{
    LONG lRetVal ;
    lRetVal = RegQueryInfoKey ( hExecutorKey , 
                                NULL , 
                                NULL , 
                                NULL , 
                                &m_cRegisteredHotkeys , 
                                NULL , 
                                NULL , 
                                NULL , 
                                NULL , 
                                NULL , 
                                NULL , 
                                NULL ) ;
    if ( ERROR_SUCCESS != lRetVal )
    {
        DbgPrintWin32Error ( lRetVal , IDS_ERROR_NO_EXECUTOR_KEY ) ;
        return FALSE ;
    }

    m_pHotkeys = new HOTKEY_INFO [ m_cRegisteredHotkeys ] ;
    if ( NULL == m_pHotkeys )
        return FALSE ;

    return TRUE ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Reads hotkey combination settings from the specified registry key and registers it in the system.
BYTE CShellExecutor::RegisterHotkey ( HKEY hExecutorKey , 
                                      LPTSTR szRegKeyName , 
                                      FPN_REGISTER_HOTKEY fpnRegisterHotkey ) 
{
    DWORD dwModKeys ;
    DWORD dwMainKey ;

    // Read async keys mask
    DWORD dwBufSize = sizeof ( DWORD ) ;
    LONG lRetVal = RegGetValue ( hExecutorKey , 
                                 szRegKeyName , 
                                 g_szAsyncKeyRegVal , 
                                 RRF_RT_REG_DWORD ,
                                 NULL ,
                                 &dwModKeys ,
                                 &dwBufSize ) ;
    if ( ERROR_SUCCESS != lRetVal )
    {
        DbgPrintWin32Error ( lRetVal , IDS_ERROR_ASYNC_KEY_MASK_READ , szRegKeyName ) ;
        return FALSE ;
    }

    // Read main key code
    dwBufSize = sizeof ( DWORD ) ;
    lRetVal = RegGetValue ( hExecutorKey , 
                            szRegKeyName , 
                            g_szMainKeyRegVal , 
                            RRF_RT_REG_DWORD ,
                            NULL ,
                            &dwMainKey ,
                            &dwBufSize ) ;
    if ( ERROR_SUCCESS != lRetVal )
    {
        DbgPrintWin32Error ( lRetVal , IDS_ERROR_MAIN_KEY_READ , szRegKeyName ) ;
        return FALSE ;
    }

    return fpnRegisterHotkey ( m_bHandlerID , dwModKeys , dwMainKey ) ;
}