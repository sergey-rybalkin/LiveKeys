#include "StdAfx.h"
#include "LayoutSwitcher.h"

#define RUSSIAN_LAYOUT 0x04190419
#define ENGLISH_LAYOUT 0x04090409

CLayoutSwitcher::CLayoutSwitcher()
{
}

CLayoutSwitcher::~CLayoutSwitcher()
{
}

// This method will be called before handler instance will be used. 
// bHandlerID - application-wide unique identifier of this handler instance
// hwndMainWindow - handler of the main application window
// hInstance - handler of this application instance
// fpnRegisterHotkey - pointer to the function that should be used to register hotkeys
// Returns TRUE if initialized successfully, FALSE otherwise.
BOOL CLayoutSwitcher::Initialize(
    BYTE bHandlerID,
    HWND hwndMainWindow,
    HINSTANCE hInstance,
    FPN_REGISTER_HOTKEY fpnRegisterHotkey)
{
    CLiveKeysHandler::Initialize(bHandlerID, hwndMainWindow, hInstance, fpnRegisterHotkey);

    return TRUE;
}

// This method will be called before application exit.
VOID CLayoutSwitcher::ShutDown(FPN_UNREGISTER_HOTKEY fpnUnregisterHotkey)
{
    CLiveKeysHandler::ShutDown(fpnUnregisterHotkey);
}

// Is called from low level keyboard hook routine to notify about keydown event, return TRUE to
// hide event from the system.
BOOL CLayoutSwitcher::NotifyKeyDown(DWORD vkCode)
{
    m_dwPrevKey = vkCode;

    return FALSE;
}

// Is called from low level keyboard hook routine to notify about keyup event, return TRUE to
// hide event from the system.
BOOL CLayoutSwitcher::NotifyKeyUp(DWORD vkCode)
{
    if ((m_dwPrevKey & vkCode) == VK_RCONTROL)
        SetLayout((HKL)RUSSIAN_LAYOUT);
    else if ((m_dwPrevKey & vkCode) == VK_LCONTROL)
        SetLayout((HKL)ENGLISH_LAYOUT);

    m_dwPrevKey = 0;

    return FALSE;
}

// Does all the work - switches layout in the foreground window.
VOID CLayoutSwitcher::SetLayout(HKL dwLayout)
{
    GUITHREADINFO Gti = { 0 };
    Gti.cbSize = sizeof(GUITHREADINFO);
    GetGUIThreadInfo(NULL, &Gti);

    if (NULL == Gti.hwndFocus)
        return;

    DWORD dwThreadId = GetWindowThreadProcessId(Gti.hwndFocus, 0);
    HKL hCurLayout = GetKeyboardLayout(dwThreadId);

    // If the specified layout is already activated then we don't need to do anything.
    if (dwLayout == hCurLayout)
        return;

    // Emulate Win + Space keyboard shortcut that switches languages regardless of the system settings
    INPUT inputs[4] = { 0 };

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_LWIN;

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_SPACE;

    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = VK_SPACE;
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_LWIN;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(4, inputs, sizeof(INPUT));
}
