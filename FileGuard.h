#pragma once
#include "StdAfx.h"
#include "LiveKeysHandler.h"

// Monitors file system for files / directories list and deletes them as soon as they are created.
class CFileGuard : public CLiveKeysHandler
{
public:
    CFileGuard();
    ~CFileGuard();

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

    // This method will be called before application exit.
    VOID ShutDown(FPN_UNREGISTER_HOTKEY fpnUnregisterHotkey);
private:
    HINSTANCE m_hInstance;
    HWND m_hwndMainWindow;
};
