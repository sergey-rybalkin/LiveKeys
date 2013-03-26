// ShellExtenderTests.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define WINDOWCLASS_NAME L"LIVEKEYS"
HINSTANCE g_hInst = NULL ;
HWND      g_hWnd  = NULL ;
HWND      g_hNextViewer = NULL ;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Handles all events that we will receive
LRESULT CALLBACK WindowProc ( HWND hWnd , UINT uMsg , WPARAM wParam , LPARAM lParam )
{
    switch ( uMsg )
    {
        case WM_CREATE :
            g_hNextViewer = SetClipboardViewer ( hWnd ) ;
            return 0 ;

        case WM_COMMAND :
            return 0 ;

        case WM_CLOSE :
            PostQuitMessage ( 0 ) ;
            return 0 ;

        case WM_CHANGECBCHAIN : 
            if ( ( HWND ) wParam == g_hNextViewer ) 
                g_hNextViewer = ( HWND ) lParam; 
            else if ( g_hNextViewer != NULL )
                SendMessage ( g_hNextViewer , uMsg , wParam , lParam ) ; 
            break ;

        case WM_DRAWCLIPBOARD :
            if ( OpenClipboard ( hWnd ) )
            {
                HGLOBAL hClipboard = GetClipboardData ( CF_UNICODETEXT ) ;
                WCHAR* pszSrc = ( WCHAR* ) GlobalLock ( hClipboard ) ;
                OutputDebugStringW ( pszSrc ) ;
                GlobalUnlock ( hClipboard ) ;

                CloseClipboard ( ) ;
            }
            if ( NULL != g_hNextViewer )
                SendMessage ( g_hNextViewer , uMsg , wParam , lParam ) ; 
            break ;

        default :
            return DefWindowProc ( hWnd , uMsg , wParam , lParam ) ;
    }
}

int _tmain(int argc, _TCHAR* argv[])
{
    g_hInst = GetModuleHandle ( NULL ) ;

    // Create a fake window to listen to events.
    WNDCLASSEX wclx =  { 0 } ;
    wclx.cbSize         = sizeof( wclx ) ;
    wclx.lpfnWndProc    = &WindowProc ;
    wclx.hInstance      = g_hInst ;
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

    MSG msg ;
    while ( GetMessage ( &msg , NULL , 0 , 0 ) )
    {
        TranslateMessage ( &msg ) ;
        DispatchMessage ( &msg ) ;
    }

    ChangeClipboardChain ( g_hWnd , g_hNextViewer ) ;

    UnregisterClass ( WINDOWCLASS_NAME , g_hInst ) ;
    DestroyWindow ( g_hWnd ) ;

	return 0 ;
}