/* -------------------------------------------------------------------------------------------------
 * ClipboardHistory.cpp - implementation file for CClipboardHistory hotkeys handler that stores 
 * windows clipboard history and allows fast access to it.
 *
 * Shell Extender (livekeys) utility for Windows Vista x86/x64
 * Copyright (c) 2009 Sergey Rybalkin - rybalkinsp@gmail.com
 ------------------------------------------------------------------------------------------------ */
#include "StdAfx.h"
#include "utils.h"
#include "ClipboardHistory.h"

CClipboardHistory::CClipboardHistory ( )
{
    m_hwndNextChain = NULL ;
    m_hwndMainWindow = NULL ;
    m_bShowHistoryHotkeyID = 0 ;
    m_pFirst = NULL ;
    m_bCurrentHistoryDepth = 0 ;
}

CClipboardHistory::~CClipboardHistory ( )
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
// This method will be called before handler instance will be used. 
// bHandlerID - application-wide unique identifier of this handler instance
// hwndMainWindow - handler of the main application window
// hInstance - handler of this application instance
// fpnRegisterHotkey - pointer to the function that should be used to register hotkeys
// Returns TRUE if initialized successfully, FALSE otherwise.
BOOL CClipboardHistory::Initialize ( BYTE bHandlerID , 
                                     HWND hwndMainWindow , 
                                     HINSTANCE hInstance ,
                                     FPN_REGISTER_HOTKEY fpnRegisterHotkey )
{
    CLiveKeysHandler::Initialize ( bHandlerID , hwndMainWindow , hInstance , fpnRegisterHotkey ) ;

    m_hwndMainWindow = hwndMainWindow ; // will need this later to open clipboard

    SetLastError ( ERROR_SUCCESS ) ;
    m_hwndNextChain = SetClipboardViewer ( hwndMainWindow ) ;
    // When m_hwndNextChain is NULL it does not mean error, so we need to check this ourself
    DWORD dwLastError = GetLastError ( ) ;
    bool bClipboardViewerRegistered = ERROR_SUCCESS == dwLastError ;

    // Register Win+V combination that will be used to show popup menu with previous clipboard 
    // content
	m_bShowHistoryHotkeyID = fpnRegisterHotkey ( bHandlerID , MOD_CONTROL | MOD_WIN , VkKeyScan ( L'v' ) ) ;

    return ( bClipboardViewerRegistered && m_bShowHistoryHotkeyID ) ? TRUE : FALSE ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Is called every time WM_HOTKEY window message is received. We have only one hotkey - the one that
// displays popup menu with the recent clipboard history items listed. Once user selects something
// from this menu we will paste it into the windows clipboard.
VOID CClipboardHistory::HandleHotkey ( BYTE bHotkeyID , DWORD dwHotkey )
{
    UNREFERENCED_PARAMETER ( dwHotkey ) ;
    UNREFERENCED_PARAMETER ( bHotkeyID ) ;

    // Check whether we actually have some history to display
    if ( NULL == m_pFirst )
        return ;

    // This menu will be displayed
    HMENU hMenu = CreatePopupMenu ( ) ;
    if ( NULL == hMenu )
        return ;

    // List all the history points in this menu, but truncate items that are longer then
    // MAX_MENU_ITEM_LENGTH letters
    ClipboardHistoryPoint* pItem = m_pFirst ;
    for ( BYTE index=0 ; index < m_bCurrentHistoryDepth ; index++ )
    {
        if ( NULL == pItem )
            break ;

        WCHAR szItemTitle [ MAX_MENU_ITEM_LENGTH ] ;
        HRESULT hr = StringCchCopy ( szItemTitle , MAX_MENU_ITEM_LENGTH , pItem->lpszContent ) ;

        // Even if buffer is too small and string is truncated - that is ok for us
        if ( SUCCEEDED ( hr ) || ( STRSAFE_E_INSUFFICIENT_BUFFER == hr ) )
        {
            WCHAR szItemText [ MAX_MENU_ITEM_LENGTH + 6 ] ;
            hr = StringCchPrintf ( szItemText , MAX_MENU_ITEM_LENGTH + 6 , L"%s\t(&%d)" , szItemTitle , index + 1 ) ;
            if ( FAILED ( hr ) )
                continue ;

            BOOL bRetVal = AppendMenuW ( hMenu , 
                                         MF_ENABLED | MF_STRING , 
                                         ( UINT_PTR ) index , 
                                         szItemText ) ;
            if ( !bRetVal )
            {
                DbgPrintWin32Error ( GetLastError ( ) , 0 ) ;
            }
        }

        pItem = pItem->pPrevious ;
    }

    // Menu should be displayed right below the cursor, so we need to know it's position
    POINT pt ;
    if ( !GetCursorPos ( &pt ) )
    {
        pt.x = 0 ;
        pt.y = 0 ;
    }

    // Save handle to the current foreground window cause we will need to make it foreground once
    // again after menu is closed - windows does not do this itself
    HWND hForegroundWindow = GetForegroundWindow ( ) ;
    
    // This required for the menu to get the focus - http://support.microsoft.com/kb/135788
    SetForegroundWindow ( m_hwndMainWindow ) ; 

    BOOL bSelectedItem = TrackPopupMenu ( 
        hMenu , 
        TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTBUTTON , 
        pt.x ,
        pt.y ,
        0 , 
        m_hwndMainWindow , 
        NULL ) ;

    DestroyMenu ( hMenu ) ;

    if ( NULL != hForegroundWindow )
        SetForegroundWindow ( hForegroundWindow ) ;

    // Make sure that something was selected
    if ( bSelectedItem >= 0 )
        PasteHistoryPoint ( ( BYTE ) bSelectedItem ) ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// This method will be called before application exit.
VOID CClipboardHistory::ShutDown ( FPN_UNREGISTER_HOTKEY fpnUnregisterHotkey )
{
    CLiveKeysHandler::ShutDown ( fpnUnregisterHotkey ) ;

    // History items linked list cleanup. Walk through all items starting from root and free memory
    if ( NULL != m_pFirst )
    {
        ClipboardHistoryPoint* pChain = m_pFirst ;
        while ( NULL != pChain )
        {
            ClipboardHistoryPoint* pPrevious = pChain->pPrevious ;

            if ( NULL != pChain->lpszContent )
                delete[] pChain->lpszContent ;
            delete pChain ;
            pChain = pPrevious ;
        }
    }

    ChangeClipboardChain ( m_hwndMainWindow , m_hwndNextChain ) ;
    fpnUnregisterHotkey ( m_bHandlerID , m_bShowHistoryHotkeyID ) ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Get accessor for m_hwndNextChain
HWND CClipboardHistory::GetNextWindowInChain ( )
{
    return m_hwndNextChain ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Set accessor for m_hwndNextChain
VOID CClipboardHistory::SetNextWindowInChain ( HWND hwndNextChain )
{
    m_hwndNextChain = hwndNextChain ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Is called when main window receives WM_DRAWCLIPBOARD message. It means that clipboard content has
// changed and we need to save it.
VOID CClipboardHistory::NotifyDrawClipboard ( UINT uMsg , WPARAM wParam , LPARAM lParam )
{
    if ( GetClipboardOwner ( ) == m_hwndMainWindow )
        return ; // ignore ourselves

    // Check whether clipboard is available and it's content is text
    if ( IsClipboardFormatAvailable ( CF_UNICODETEXT ) && OpenClipboard ( m_hwndMainWindow ) )
    {
        // Retrieve the text from clipboard
        HGLOBAL hClipboard = GetClipboardData ( CF_UNICODETEXT ) ;
        WCHAR* pszSrc = ( WCHAR* ) GlobalLock ( hClipboard ) ;

        // Check whether it's not too big, if it is - just skip it
        size_t nSize = 0 ;
        HRESULT hr = StringCbLength ( pszSrc , MAX_CLIPBOARD_LEN , &nSize ) ;
        if ( SUCCEEDED ( hr ) )
        {
            if ( MAX_CLIPBOARD_LEN > nSize )
                PushHistoryPoint ( pszSrc , nSize ) ;
        }

        GlobalUnlock ( hClipboard ) ;

        if ( !CloseClipboard ( ) )
        {
            DbgPrintWin32Error ( GetLastError ( ) , NULL ) ;
        }
    }

    if ( NULL != m_hwndNextChain )
        SendMessage ( m_hwndNextChain , uMsg , wParam , lParam ) ; 
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Puts the specified content into clipboard history and removes last element if history size
// exceeds it's maximum depth.
VOID CClipboardHistory::PushHistoryPoint ( WCHAR* lpszClipboardContent , size_t cbLen ) 
{
    m_bCurrentHistoryDepth++ ;

    // If history linked list exceeds it's maximum size - remove the oldest element
    if ( MAX_HISTORY_DEPTH <= m_bCurrentHistoryDepth )
    {
        ClipboardHistoryPoint* pLast = m_pFirst ;
        while ( NULL != pLast->pPrevious->pPrevious )
            pLast = pLast->pPrevious ;

        delete[] pLast->pPrevious->lpszContent ;
        delete pLast->pPrevious ;
        pLast->pPrevious = NULL ;

        m_bCurrentHistoryDepth-- ;
    }

    // Add new history point at the beginning of the linked list
    ClipboardHistoryPoint* pNewRoot = new ClipboardHistoryPoint ( ) ;
    pNewRoot->lpszContent = new WCHAR [ cbLen + sizeof ( WCHAR ) ] ;
    HRESULT hr = StringCbCopy ( pNewRoot->lpszContent , 
                                cbLen + sizeof ( WCHAR ) , 
                                lpszClipboardContent ) ;
    if ( SUCCEEDED ( hr ) )
    {
        pNewRoot->pPrevious = m_pFirst ;
        m_pFirst = pNewRoot ;
    }
    else
    {
        delete[] pNewRoot->lpszContent ;
        delete pNewRoot ;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Pastes text from the history linked list specified by bIndex into the windows clipboard
VOID CClipboardHistory::PasteHistoryPoint ( BYTE bIndex ) 
{
    // Seek to the required item and make sure it actually exists.
    ClipboardHistoryPoint* pItem = m_pFirst ;
    for ( BYTE index = 0 ; index != bIndex ; index++ )
    {
        if ( ( NULL == pItem ) || ( NULL == pItem->lpszContent ) )
            return ;
        pItem = pItem->pPrevious ;
    }
    
    if ( !OpenClipboard ( m_hwndMainWindow ) )
    {
        DbgPrintWin32Error ( GetLastError ( ) , 0 ) ;
        return ;
    }

    BOOL bRetVal = EmptyClipboard ( ) ; // This is required before updating clipboard
    if ( bRetVal )
    {
        // Calculate the size of the string to paste
        size_t nSize = 0 ;
        HRESULT hr = StringCchLength ( pItem->lpszContent , MAX_CLIPBOARD_LEN , &nSize ) ;
        if ( FAILED ( hr ) )
        {
            CloseClipboard ( ) ;
            return ;
        }

        // SetClipboardData needs a global memory pointer so we cannot just StringCchCopy text to
        // the clipboard. First alloc global buffer, write our text to it and then put it into the
        // clipboard.
        HGLOBAL hglbCopy = GlobalAlloc ( GMEM_MOVEABLE , sizeof ( WCHAR ) * ( nSize + 1 ) ) ;
        if ( NULL == hglbCopy )
        {
            CloseClipboard ( ) ;
            return ;
        }

        WCHAR* pszDest = ( WCHAR* ) GlobalLock ( hglbCopy ) ;
        hr = StringCchCopy ( pszDest , nSize + 1 , pItem->lpszContent ) ;
        GlobalUnlock ( hglbCopy ) ;

        if ( FAILED ( hr ) )
        {
            GlobalFree ( hglbCopy ) ;
            CloseClipboard ( ) ;
            return ;
        }

        SetClipboardData ( CF_UNICODETEXT , hglbCopy ) ;
    }
    else
        DbgPrintWin32Error ( GetLastError ( ) , 0 ) ;

    bRetVal = CloseClipboard ( ) ;

    if ( !bRetVal )
    {
        DbgPrintWin32Error ( GetLastError ( ) , 0 ) ;
    }
}