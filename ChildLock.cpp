#include "stdafx.h"
#include "utils.h"
#include "ChildLock.h"

CChildLock::CChildLock()
{
    m_hInstance = nullptr;
    m_hwndMainWindow = nullptr;
    m_bLockInputHotkeyID = 0;
}

CChildLock::~CChildLock()
{
}

// This method will be called before handler instance will be used. 
// bHandlerID - application-wide unique identifier of this handler instance
// hwndMainWindow - handler of the main application window
// hInstance - handler of this application instance
// fpnRegisterHotkey - pointer to the function that should be used to register hotkeys
// Returns TRUE if initialized successfully, FALSE otherwise.
BOOL CChildLock::Initialize(
    BYTE bHandlerID,
    HWND hwndMainWindow,
    HINSTANCE hInstance,
    FPN_REGISTER_HOTKEY fpnRegisterHotkey)
{
    CLiveKeysHandler::Initialize(bHandlerID, hwndMainWindow, hInstance, fpnRegisterHotkey);

    m_hInstance = hInstance;
    m_hwndMainWindow = hwndMainWindow;

    m_bLockInputHotkeyID = fpnRegisterHotkey(bHandlerID, MOD_CONTROL | MOD_WIN, VkKeyScan(L'b'));

    return TRUE;
}

// Is called every time WM_HOTKEY window message is received. We have only one hotkey - the one that
// blocks keyboard and mouse input.
VOID CChildLock::HandleHotkey(BYTE bHotkeyID, DWORD dwHotkey)
{
    UNREFERENCED_PARAMETER(dwHotkey);

    if (m_bLockInputHotkeyID != bHotkeyID)
        return;

    BlockInput(TRUE);
}

VOID CChildLock::ShutDown(FPN_UNREGISTER_HOTKEY fpnUnregisterHotkey)
{
    CLiveKeysHandler::ShutDown(fpnUnregisterHotkey);
}