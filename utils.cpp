/* -------------------------------------------------------------------------------------------------
 * utils.cpp - implementation file for a set of utility functions used through the whole 
 * application.
 *
 * Shell Extender (livekeys) utility for Windows Vista x86/x64
 * Copyright (c) 2009 Sergey Rybalkin - rybalkinsp@gmail.com
 ------------------------------------------------------------------------------------------------ */
#include "StdAfx.h"
#include "utils.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Reports custom application error to the user
VOID ShowCustomError( UINT uiCaptionStringID , UINT uiMsgStringID )
{
    // Load custom strings if specified
    TCHAR szCaption [ 256 ] ;
    TCHAR szCustomMsg [ 256 ] ;
    int iRetVal = LoadString ( g_hInst , uiCaptionStringID , (LPTSTR) &szCaption , 256 ) ;
    if ( 0 == iRetVal )
        StringCchCopy ( szCaption , 256 , TEXT ( "Caption too big" ) ) ;
    iRetVal = LoadString ( g_hInst , uiMsgStringID , (LPTSTR) &szCustomMsg , 256 ) ;
    if ( 0 == iRetVal )
        StringCchCopy ( szCaption , 256 , TEXT ( "Caption too big" ) ) ;

    // Show custom error message if specified, then show system error message.
    if ( szCustomMsg && szCaption )
        MessageBox ( NULL , szCustomMsg , szCaption , MB_OK | MB_ICONERROR ) ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Translates a Win32 error into a text equivalent
VOID ShowWin32Error( DWORD dwErrorCode , UINT uiCaptionStringID , UINT uiMsgStringID )
{
    // Format system error message
    LPTSTR szSystemMsg ;
    FormatMessage ( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM ,
                    NULL ,
                    dwErrorCode ,
                    MAKELANGID ( LANG_NEUTRAL , SUBLANG_DEFAULT ) ,
                    (LPTSTR) &szSystemMsg ,
                    0 , 
                    NULL );

    // Load custom strings if specified
    TCHAR* szCaption = 0 ;
    TCHAR* szCustomMsg = 0 ;
    if ( uiCaptionStringID > 0 )
        LoadString ( g_hInst , uiCaptionStringID , (LPTSTR) &szCaption , 0 ) ;
    if ( uiMsgStringID > 0 )
        LoadString ( g_hInst , uiMsgStringID , (LPTSTR) &szCustomMsg , 0 ) ;

    // Show custom error message if specified, then show system error message.
    if ( szCustomMsg )
        MessageBox ( NULL , szCustomMsg , szCaption , MB_OK | MB_ICONERROR ) ;
    MessageBox ( NULL , szSystemMsg , szCaption , MB_OK | MB_ICONERROR ) ;

    LocalFree ( szSystemMsg ) ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Silently prints text equivalent of Win32 error into a diagnostics buffer.
#ifdef _DEBUG

VOID DbgPrintWin32Error( DWORD dwErrorCode , UINT uiMsgStringID , ... )
{
    va_list args ;
    va_start ( args , uiMsgStringID ) ;

    // Format system error message
    LPTSTR szSystemMsg ;
    FormatMessage ( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM ,
                    NULL ,
                    dwErrorCode ,
                    MAKELANGID ( LANG_NEUTRAL , SUBLANG_DEFAULT ) ,
                    (LPTSTR) &szSystemMsg ,
                    0 , 
                    NULL );
    // Load custom strings if specified
    TCHAR* szCustomMsgFmt = 0 ;
    TCHAR szCustomMsg [ 2048 ] = { 0 } ;

    if ( uiMsgStringID > 0 )
    {
        LoadString ( g_hInst , uiMsgStringID , (LPTSTR) &szCustomMsgFmt , 0 ) ;
        HRESULT hr = StringCchVPrintf ( szCustomMsg , 2048 , szCustomMsgFmt , args ) ;
        if ( FAILED ( hr ) )
            return ; // message is too long, just skip it
    }

    // Print custom error message if specified, then print system error message.
    if ( szCustomMsg )
        OutputDebugString ( szCustomMsg ) ;
    OutputDebugString ( szSystemMsg ) ;

    LocalFree ( szSystemMsg ) ;

    va_end ( args ) ;
}

#endif