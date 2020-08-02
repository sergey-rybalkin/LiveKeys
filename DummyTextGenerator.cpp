#include "StdAfx.h"
#include "utils.h"
#include "DummyTextGenerator.h"

CDummyTextGenerator::CDummyTextGenerator()
{
    m_hInstance = NULL;
    m_bShowDummyTextMenuHotkeyID = 0;
    m_hwndMainWindow = NULL;
}

CDummyTextGenerator::~CDummyTextGenerator()
{
}

// This method will be called before handler instance will be used. 
// bHandlerID - application-wide unique identifier of this handler instance
// hwndMainWindow - handler of the main application window
// hInstance - handler of this application instance
// fpnRegisterHotkey - pointer to the function that should be used to register hotkeys
// Returns TRUE if initialized successfully, FALSE otherwise.
BOOL CDummyTextGenerator::Initialize(
    BYTE bHandlerID,
    HWND hwndMainWindow,
    HINSTANCE hInstance,
    FPN_REGISTER_HOTKEY fpnRegisterHotkey)
{
    CLiveKeysHandler::Initialize(bHandlerID, hwndMainWindow, hInstance, fpnRegisterHotkey);

    m_hInstance = hInstance;
    m_hwndMainWindow = hwndMainWindow;

    m_bShowDummyTextMenuHotkeyID = fpnRegisterHotkey(bHandlerID, MOD_CONTROL | MOD_WIN, VkKeyScan(L'x'));

    return m_bShowDummyTextMenuHotkeyID > 0;
}

// Is called every time WM_HOTKEY window message is received. We have only one hotkey - the one that
// displays popup menu with the recent clipboard history items listed. Once user selects something
// from this menu we will paste it into the windows clipboard.
VOID CDummyTextGenerator::HandleHotkey(BYTE bHotkeyID, DWORD dwHotkey)
{
    UNREFERENCED_PARAMETER(dwHotkey);

    if (m_bShowDummyTextMenuHotkeyID != bHotkeyID)
        return;
    HMENU hMenu = CreatePopupMenu();
    if (NULL == hMenu)
        return;

    AppendMenuW(hMenu, MF_ENABLED | MF_STRING, 0, L"Lipsum 1");
    AppendMenuW(hMenu, MF_ENABLED | MF_STRING, 1, L"Lipsum 2");
    AppendMenuW(hMenu, MF_ENABLED | MF_STRING, 2, L"Lipsum 3");
    AppendMenuW(hMenu, MF_ENABLED | MF_STRING, 3, L"Lipsum 4");

    // Menu should be displayed right below the cursor, so we need to know it's position
    POINT pt;
    if (!GetCursorPos(&pt))
    {
        pt.x = 0;
        pt.y = 0;
    }

    // Save handle to the current foreground window cause we will need to make it foreground once
    // again after menu is closed - windows does not do this itself
    HWND hForegroundWindow = GetForegroundWindow();

    // This required for the menu to get the focus - http://support.microsoft.com/kb/135788
    SetForegroundWindow(m_hwndMainWindow);

    BOOL bSelectedItem = TrackPopupMenu(
        hMenu,
        TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTBUTTON,
        pt.x,
        pt.y,
        0,
        m_hwndMainWindow,
        NULL);

    DestroyMenu(hMenu);

    if (NULL != hForegroundWindow)
        SetForegroundWindow(hForegroundWindow);

    WCHAR* szDummyText = NULL;
    int iBufSize = 0;

    switch (bSelectedItem)
    {
    case 0:
        iBufSize = LoadString(m_hInstance, IDS_LIPSUM1, (LPWSTR)&szDummyText, 0);
        break;
    case 1:
        iBufSize = LoadString(m_hInstance, IDS_LIPSUM2, (LPWSTR)&szDummyText, 0);
        break;
    case 2:
        iBufSize = LoadString(m_hInstance, IDS_LIPSUM3, (LPWSTR)&szDummyText, 0);
        break;
    case 3:
        iBufSize = LoadString(m_hInstance, IDS_LIPSUM4, (LPWSTR)&szDummyText, 0);
        break;
    default:
        return;
    }

    if (iBufSize < 1)
        return;

    INPUT* pInputs = new INPUT[iBufSize * 2];
    ZeroMemory(pInputs, iBufSize * 2 * sizeof(INPUT));

    for (int index = 0; index < iBufSize * 2; index++)
    {
        INPUT* input = pInputs + index;

        input->type = INPUT_KEYBOARD;

        if ((index > 0) && (index % 2))
            input->ki.dwFlags = KEYEVENTF_KEYUP;

        input->ki.wVk = VkKeyScanW(szDummyText[index / 2]);
    }

    ::SendInput(iBufSize * 2, pInputs, sizeof(INPUT));

    delete[] pInputs;
}

// This method will be called before application exit.
VOID CDummyTextGenerator::ShutDown(FPN_UNREGISTER_HOTKEY fpnUnregisterHotkey)
{
    CLiveKeysHandler::ShutDown(fpnUnregisterHotkey);

    fpnUnregisterHotkey(m_bHandlerID, m_bShowDummyTextMenuHotkeyID);
}
