#pragma once
#include "LiveKeysHandler.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Constants
#define HOTKEY_WIN   1
#define HOTKEY_CTRL  2
#define HOTKEY_ALT   4
#define HOTKEY_SHIFT 8

////////////////////////////////////////////////////////////////////////////////////////////////////
// Defines hotkey information container
typedef struct
{
    BYTE   bHotkeyID;
    LPTSTR lpszItemName;
} HOTKEY_INFO, * PHOTKEY_INFO;

class CShellExecutor : public CLiveKeysHandler
{
public:
    CShellExecutor();
    ~CShellExecutor();

    // This method will be called before handler instance will be used. 
    // bHandlerID - application-wide unique identifier of this handler instance
    // hwndMainWindow - handler of the main application window
    // hInstance - handler of this application instance
    // fpnRegisterHotkey - pointer to the function that should be used to register hotkeys
    // Returns TRUE if initialized successfully, FALSE otherwise.
    BOOL Initialize(
        BYTE bHandlerID,
        HWND hwndMainWindow,
        HINSTANCE hInstance,
        FPN_REGISTER_HOTKEY fpnRegisterHotkey);

    // Is called every time WM_HOTKEY window message is received.
    VOID HandleHotkey(BYTE bHotkeyID, DWORD dwHotkey);

    // This method will be called before application exit.
    VOID ShutDown(FPN_UNREGISTER_HOTKEY fpnUnregisterHotkey);

private:
    BOOL AllocateMemoryForHotkeys(HKEY hExecutorKey);
    BYTE RegisterHotkey(HKEY hExecutorKey,
        LPTSTR szRegKeyName,
        FPN_REGISTER_HOTKEY fpnRegisterHotkey);

    DWORD        m_cRegisteredHotkeys; // stores the number of registered hotkeys
    HOTKEY_INFO* m_pHotkeys; // pointer to the array of hotkeys managed by the current instance
};
