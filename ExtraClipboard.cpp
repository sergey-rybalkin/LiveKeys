/* -------------------------------------------------------------------------------------------------
 * ExtraClipboard.h - implementation file for CExtraClipboard hotkeys handler class that adds 
 * additional clipboards to the system.
 *
 * Shell Extender (livekeys) utility for Windows Vista x86/x64
 * Copyright (c) 2009 Sergey Rybalkin - rybalkinsp@gmail.com
 ------------------------------------------------------------------------------------------------ */
#include "StdAfx.h"
#include "ExtraClipboard.h"
#include "utils.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Globals and constants
#define VK_KEY_FIRST_CLIPBOARD VK_NUMPAD1 // Numpad key for the first clipboard
#define VK_KEY_LAST_CLIPBOARD  VK_NUMPAD1 + NUM_CLIPBOARDS // Numpad key for the last clipboard
#define MAX_CLIPBOARD_CAPACITY 102410

CExtraClipboard::CExtraClipboard ( )
{
    ZeroMemory ( m_pszClipboardsContent , sizeof ( INT_PTR ) * NUM_CLIPBOARDS ) ;
}

CExtraClipboard::~CExtraClipboard ( )
{
    for ( int index = 0 ; index < NUM_CLIPBOARDS ; index++ )
    {
        if ( NULL !=  m_pszClipboardsContent [ index ] )
           delete[] m_pszClipboardsContent [ index ] ;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// This method will be called before handler instance will be used. 
// bHandlerID - application-wide unique identifier of this handler instance
// hwndMainWindow - handler of the main application window
// hInstance - handler of this application instance
// fpnRegisterHotkey - pointer to the function that should be used to register hotkeys
// Returns TRUE if initialized successfully, FALSE otherwise.
BOOL CExtraClipboard::Initialize ( BYTE bHandlerID , 
                                   HWND hwndMainWindow , 
                                   HINSTANCE hInstance ,
                                   FPN_REGISTER_HOTKEY fpnRegisterHotkey )
{
    CLiveKeysHandler::Initialize ( bHandlerID , hwndMainWindow , hInstance , fpnRegisterHotkey ) ;

    // NUM_CLIPBOARDS * 2 hotkeys needs to be registered - for both copy and paste actions
    for ( UINT vkHotkey = VK_KEY_FIRST_CLIPBOARD ; vkHotkey <= VK_KEY_LAST_CLIPBOARD ; vkHotkey++ )
    {
        UINT uiArrayIndex = vkHotkey - VK_KEY_FIRST_CLIPBOARD ;

        m_pbCopyHotkeys [ uiArrayIndex ] = 
            fpnRegisterHotkey ( bHandlerID , MOD_CONTROL , vkHotkey ) ;
        m_pbPasteHotkeys [ uiArrayIndex ] = 
            fpnRegisterHotkey ( bHandlerID , MOD_ALT , vkHotkey ) ;

        if ( !m_pbCopyHotkeys [ uiArrayIndex ] || !m_pbPasteHotkeys [ uiArrayIndex ] )
            return FALSE ;
    }

    return TRUE ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// This method will be called before application exit.
VOID CExtraClipboard::ShutDown ( FPN_UNREGISTER_HOTKEY fpnUnregisterHotkey )
{
    CLiveKeysHandler::ShutDown ( fpnUnregisterHotkey ) ;

    for ( UINT vkHotkey = VK_KEY_FIRST_CLIPBOARD ; vkHotkey <= VK_KEY_LAST_CLIPBOARD ; vkHotkey++ )
    {
        UINT uiArrayIndex = vkHotkey - VK_KEY_FIRST_CLIPBOARD ;

        fpnUnregisterHotkey ( m_bHandlerID , m_pbCopyHotkeys [ uiArrayIndex ] ) ;
        fpnUnregisterHotkey ( m_bHandlerID , m_pbPasteHotkeys [ uiArrayIndex ] ) ;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Is called every time WM_HOTKEY window message is received. We have only one hotkey - the one that
// displays popup menu with the recent clipboard history items listed. Once user selects something
// from this menu we will paste it into the windows clipboard.
VOID CExtraClipboard::HandleHotkey ( BYTE bHotkeyID , DWORD dwHotkey )
{
    UNREFERENCED_PARAMETER ( dwHotkey ) ;
    UNREFERENCED_PARAMETER ( bHotkeyID ) ;

    for ( int index = 0 ; index < NUM_CLIPBOARDS ; index++ )
    {
        if ( bHotkeyID == m_pbCopyHotkeys [ index ] )
        {
            CopyToExtraClipboard ( index ) ;
            return ;
        }
        else if ( bHotkeyID == m_pbPasteHotkeys [ index ] )
        {
            PateFromExtraClipboard ( index ) ;
            return ;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Copies selected text in the active window to the extra clipboard specified by iClipboardIndex.
VOID CExtraClipboard::CopyToExtraClipboard ( int iClipboardIndex )
{
    // Check whether the specified clipboard is already occupied and needs to be cleared
    if ( NULL != m_pszClipboardsContent [ iClipboardIndex ] )
    {
        delete[] m_pszClipboardsContent [ iClipboardIndex ] ;
        m_pszClipboardsContent [ iClipboardIndex ] = NULL ;
    }

    SendCopyCmd ( ) ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Pastes text from the extra clipboard specified by iClipboardIndex into the active window.
VOID CExtraClipboard::PateFromExtraClipboard ( int iClipboardIndex )
{
    // Check if there is anything to paste
    if ( NULL == m_pszClipboardsContent [ iClipboardIndex ] )
        return ;

    SendPasteCmd ( ) ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Sends Ctrl+C combination to the input queue
VOID CExtraClipboard::SendCopyCmd ( )
{
    INPUT input = { 0 } ;
    input.type = INPUT_KEYBOARD ;
    input.ki.wScan = 0 ;
    input.ki.time = 0 ;
    input.ki.dwExtraInfo = NULL ;
    input.ki.wVk = VK_CONTROL ;
    SendInput ( 1 , &input , sizeof ( input ) ) ;

    input.ki.wVk = VkKeyScan ( 'c' ) ;
    SendInput ( 1 , &input , sizeof ( input ) ) ;

    input.ki.dwFlags = KEYEVENTF_KEYUP ;
    SendInput ( 1 , &input , sizeof ( input ) ) ;

    input.ki.wVk = VK_CONTROL ;
    SendInput ( 1 , &input , sizeof ( input ) ) ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Sends Ctrl+P combination to the input queue
VOID CExtraClipboard::SendPasteCmd ( )
{
    // At this moment Alt key is pressed, make sure system thnks it is released now
    INPUT input = { 0 } ;
    input.type = INPUT_KEYBOARD ;
    input.ki.wVk = VK_MENU ;
    input.ki.dwFlags = KEYEVENTF_KEYUP ;
    SendInput ( 1 , &input , sizeof ( INPUT ) ) ;

    // After ALT key was pressed in most applications menu is activated, so we need to press ALT 
    // once again to deactivate it
    input.ki.dwFlags = 0 ;
    SendInput ( 1 , &input , sizeof ( INPUT ) ) ;

    input.ki.dwFlags = KEYEVENTF_KEYUP ;
    SendInput ( 1 , &input , sizeof ( INPUT ) ) ;

    // And now let's send CTRL+V command
    input.ki.wVk = VK_CONTROL ;
    input.ki.dwFlags = 0 ;
    SendInput ( 1 , &input , sizeof ( INPUT ) ) ;

    input.ki.wVk = VkKeyScan ( 'v' ) ;
    SendInput ( 1 , &input , sizeof ( INPUT ) ) ;

    input.ki.dwFlags = KEYEVENTF_KEYUP ;
    SendInput ( 1 , &input , sizeof ( INPUT ) ) ;

    input.ki.wVk = VK_CONTROL ;
    SendInput ( 1 , &input , sizeof ( INPUT ) ) ;
}





























































//////////////////////////////////////////////////////////////////////////////////////////////////////
//// Copies selected text from the foreground window into the extra clipboard specified by
//// dwClipboardIndex. Algorithm:
//// 1) Assume that current clipboard data is stored into the temporary buffer - 
////    g_pszOriginalClipboardData and Ctrl+C input simulated.
//// 2) Check clipboard data format. If not available as unicode text - ignore.
//// 3) Check the requested extra clipboard status. If occupied - empty it.
//// 4) Copy system clipboard data to the requested extra clipboard.
//// 5) Insert the original content into the clipboard.
//VOID CopyToExtraClipboard ( PTP_CALLBACK_INSTANCE pInstance,
//                            PVOID pvContext,
//                            PTP_TIMER pTimer )
//{
//    AsyncEventContext* context = ( AsyncEventContext* ) pvContext ;
//    CExtraClipboard* pClipboards = context->pClipboards ;
//    DWORD dwClipboardIndex = context->dwClipboardIndex ;
//
//    if ( !IsClipboardFormatAvailable ( CF_UNICODETEXT ) )
//    {
//        if ( NULL != pszOriginalClipboardData )
//        {
//            pClipboards->SetClipboardContent ( pszOriginalClipboardData ) ;
//            HeapFree ( GetProcessHeap ( ) , 0 , pszOriginalClipboardData ) ;
//        }
//        return ;
//    }
//
//    pClipboards->SetExtraClipboardContent ( dwClipboardIndex , 
//                                            pClipboards->SaveClipboardContent ( ) ) ;
//
//    if ( NULL != pszOriginalClipboardData )
//    {
//        pClipboards->SetClipboardContent ( pszOriginalClipboardData ) ;
//        HeapFree ( GetProcessHeap ( ) , 0 , pszOriginalClipboardData ) ;
//    }
//}
//
//////////////////////////////////////////////////////////////////////////////////////////////////////
//// Pastes text from the extra clipboard specified by dwClipboardIndex into the foreground window.
//// Algorithm:
//// 1) Check the requested extra clipboard status. If empty - ignore command.
//// 2) Store current clipboard data into the temporary buffer.
//// 3) Populate system clipboard with the text from requested extra clipboard.
//// 4) Simulate Ctrl+V input.
//// 5) Insert the original content into the clipboard.
//VOID PasteFromExtraClipboard ( CExtraClipboard* pClipboards , DWORD dwClipboardIndex )
//{
//    WCHAR* pContentToPaste = pClipboards->GetExtraClipboardContent ( dwClipboardIndex ) ;
//    if ( NULL == pContentToPaste )
//        return ;
//
//    // If clipboard contains anything except for the text - information will be lost
//    WCHAR* pszOriginalClipboardData = pClipboards->SaveClipboardContent ( ) ;
//    BOOL bRetVal = pClipboards->SetClipboardContent ( pContentToPaste ) ;
//    if ( !bRetVal )
//    {
//        HeapFree ( GetProcessHeap ( ) , 0 , pszOriginalClipboardData ) ;
//        return ;
//    }
//    pClipboards->SendPasteCmd ( ) ;
//    Sleep ( 400 ) ; // wait for the foreground window to update clipboard
//
//    if ( NULL != pszOriginalClipboardData )
//    {
//        pClipboards->SetClipboardContent ( pszOriginalClipboardData ) ;
//        HeapFree ( GetProcessHeap ( ) , 0 , pszOriginalClipboardData ) ;
//    }
//}
//
//
//CExtraClipboard::CExtraClipboard ( WORD wID ) : CHotKeysHandler ( wID )
//{
//    ZeroMemory ( m_pszClipboardsContent , sizeof ( INT_PTR ) * NUM_CLIPBOARDS ) ;
//    m_pWorkItem = NULL ;
//    InitializeCriticalSection ( &m_csWorkItemLock ) ;
//}
//
//CExtraClipboard::~CExtraClipboard(void)
//{
//    DeleteCriticalSection ( &m_csWorkItemLock ) ;
//
//    for ( int index=0 ; index < NUM_CLIPBOARDS ; index++ )
//    {
//        if ( NULL !=  m_pszClipboardsContent [ index ] )
//            HeapFree ( GetProcessHeap ( ) , 0 , m_pszClipboardsContent [ index ] ) ;
//    }
//}
//
//////////////////////////////////////////////////////////////////////////////////////////////////////
//// Overrides CHotKeysHandler::HandleHotkey in order to handle Ctrl/Alt + Num Pad numbers keys
//// Depending on the requested action launches one of two helper method asyncronously. 
//BOOL CExtraClipboard::HandleHotkey ( KEYBOARD_EVENT keKey )
//{
//    // Make sure we are interested in the current combination
//    if ( ( !keKey.bAltPressed && !keKey.bCtrlPressed ) || 
//         ( VK_KEY_FIRST_CLIPBOARD > keKey.dwKeyCode ) || 
//         ( VK_KEY_LAST_CLIPBOARD < keKey.dwKeyCode ) ||
//         !keKey.bKeyDown)
//        return FALSE ;
//
//    // Index of the extra clipboard in m_pszClipboardsContent involved into current action
//    DWORD dwClipboardIndex = keKey.dwKeyCode - VK_KEY_FIRST_CLIPBOARD ;
//    if ( keKey.bCtrlPressed ) // copy action
//    {
//        // Check if we are busy at the moment. If we are - just ignore this command.
//        if ( !TryEnterCriticalSection ( &m_csWorkItemLock ) )
//            return TRUE ;
//        // Save original clipboard content for the later use, will be restored after we've finished.
//        // If clipboard contains anything except for the text - information will be lost
//        g_pszOriginalClipboardData = SaveClipboardContent ( ) ;
//        SendCopyCmd ( ) ;
//
//        m_pWorkItem = CreateThreadPoolTimer ( CopyToExtraClipboard , NULL , NULL ) ;
//        if ( NULL == m_pWorkItem )
//        {
//            LeaveCriticalSection ( &m_csWorkItemLock ) ;
//            DbgPrintWin32Error ( GetLastError ( ) ) ;
//        }
//        else
//        {
//            SetThreadPoolTimer ( m_pWorkItem , -1 , 50 , 0 ) ;
//        }
//
//        return TRUE ;
//    }
//    else if ( keKey.bAltPressed ) // paste action
//    {
//        // Check if we are busy at the moment. If we are - just ignore this command.
//        if ( !TryEnterCriticalSection ( &m_csWorkItemLock ) )
//            return TRUE ;
//
//        if ( NULL == m_pszClipboardsContent [ dwClipboardIndex ] )
//            return FALSE ;
//
//        // If clipboard contains anything except for the text - information will be lost
//        WCHAR* pszOriginalClipboardData = SaveClipboardContent ( ) ;
//        BOOL bRetVal = SetClipboardContent ( m_pszClipboardsContent [ dwClipboardIndex ] ) ;
//        if ( !bRetVal )
//        {
//            HeapFree ( GetProcessHeap ( ) , 0 , pszOriginalClipboardData ) ;
//            return FALSE ;
//        }
//        SendPasteCmd ( ) ;
//        Sleep ( 400 ) ; // wait for the foreground window to update clipboard
//
//        if ( NULL != pszOriginalClipboardData )
//        {
//            SetClipboardContent ( pszOriginalClipboardData ) ;
//
//            HeapFree ( GetProcessHeap ( ) , 0 , pszOriginalClipboardData ) ;
//        }
//    }
//
//    return TRUE ;
//}
//
//////////////////////////////////////////////////////////////////////////////////////////////////////
//// If system clipboard contains unicode text - method stores it in the heap and returns pointer to 
//// this text. Do not forget to call HeapFree for the returned pointer.
//WCHAR* CExtraClipboard::SaveClipboardContent ( )
//{
//    WCHAR* pszRetVal = NULL ;
//
//    if ( !IsClipboardFormatAvailable ( CF_UNICODETEXT ) || !OpenClipboard ( g_hWnd ) )
//    {
//        pszRetVal = NULL ;
//        return pszRetVal ;
//    }
//
//    // Clipboard is opened and contains unicode text. Get the data and store it in the heap.
//    HGLOBAL hClipboardData = GetClipboardData ( CF_UNICODETEXT ) ;
//    WCHAR* pszSrc = ( WCHAR* )GlobalLock ( hClipboardData ) ;
//    SIZE_T nLen = 0 ;
//    HRESULT hr = StringCchLength ( pszSrc , MAX_CLIPBOARD_CAPACITY , (size_t*) &nLen ) ;
//    if ( FAILED ( hr ) )
//    {
//        GlobalUnlock ( hClipboardData ) ;
//        CloseClipboard ( ) ;
//        pszRetVal = NULL ;
//        return pszRetVal ;
//    }
//
//    pszRetVal = (WCHAR*) HeapAlloc ( GetProcessHeap ( ) , 
//                                     HEAP_ZERO_MEMORY , 
//                                     ( nLen + 1 ) * sizeof ( WCHAR ) ) ;
//    if ( NULL == pszRetVal )
//    {
//        GlobalUnlock ( hClipboardData ) ;
//        CloseClipboard ( ) ;
//        return pszRetVal ;
//    }
//
//    CopyMemory ( pszRetVal , pszSrc , ( nLen + 1 ) * sizeof ( WCHAR ) ) ;
//
//    GlobalUnlock ( hClipboardData ) ;
//    CloseClipboard ( ) ;
//
//    return pszRetVal ;
//}
//

//
//////////////////////////////////////////////////////////////////////////////////////////////////////
//BOOL CExtraClipboard::SetClipboardContent ( WCHAR* pszData )
//{
//    if ( !OpenClipboard ( g_hWnd ) )
//        return FALSE ;
//
//    if ( !EmptyClipboard ( ) )
//    {
//        CloseClipboard ( ) ;
//        return FALSE ;
//    }
//
//    SIZE_T nLen = 0 ;
//    HRESULT hr = StringCchLength ( pszData , MAX_CLIPBOARD_CAPACITY , (size_t*) &nLen ) ;
//    if ( FAILED ( hr ) )
//    {
//        CloseClipboard ( ) ;
//        return FALSE ;
//    }
//
//    HGLOBAL hGlb = GlobalAlloc ( GMEM_MOVEABLE , ( nLen + 1 ) * sizeof ( WCHAR ) ) ;
//    if ( NULL == hGlb )
//    {
//        CloseClipboard ( ) ;
//        return FALSE ;
//    }
//    WCHAR* pszDest = ( WCHAR* ) GlobalLock ( hGlb ) ;
//    CopyMemory ( pszDest , pszData , ( nLen + 1 ) * sizeof ( WCHAR ) ) ;
//    GlobalUnlock ( hGlb ) ;
//    BOOL bRetVal = ( NULL != SetClipboardData ( CF_UNICODETEXT , hGlb ) ) ;
//
//    CloseClipboard ( ) ;
//
//    return bRetVal ;
//}
//
//////////////////////////////////////////////////////////////////////////////////////////////////////
//// Returns pointer to the process heap buffer that contains content of the specified extra clipboard
//WCHAR* CExtraClipboard::GetExtraClipboardContent ( DWORD dwClipboardIndex )
//{
//    if ( dwClipboardIndex < NUM_CLIPBOARDS )
//        return m_pszClipboardsContent [ dwClipboardIndex ] ;
//    else
//        return NULL ;
//}
//
//////////////////////////////////////////////////////////////////////////////////////////////////////
//// Sets content of the specified extra clipboard to the process heap buffer pointed by pszContent
//VOID CExtraClipboard::SetExtraClipboardContent ( DWORD dwClipboardIndex , WCHAR* pszContent )
//{
//    if ( dwClipboardIndex >= NUM_CLIPBOARDS )
//        return ;
//
//    if ( NULL != m_pszClipboardsContent [ dwClipboardIndex ] )
//        HeapFree ( GetProcessHeap ( ) , 0 , m_pszClipboardsContent [ dwClipboardIndex ] ) ;
//
//    m_pszClipboardsContent [ dwClipboardIndex ] = pszContent ;
//}