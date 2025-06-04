#include "stdafx.h"
#include "utils.h"
#include "FileGuard.h"
#include "shellapi.h"
#include "strsafe.h"
#include "shlobj.h"

#define NOTIFICATION_BUFFER_SIZE 16384

HANDLE m_hDir;
LPVOID m_lpNotificationBuf;
PWSTR m_pwstrTargetDirectory;

BOOL StartMonitor();

VOID CALLBACK WatchCompletion(DWORD status, DWORD bytes_ret, OVERLAPPED* io_info)
{
    UNREFERENCED_PARAMETER(status);
    UNREFERENCED_PARAMETER(bytes_ret);
    UNREFERENCED_PARAMETER(io_info);

    FILE_NOTIFY_INFORMATION* fni = (FILE_NOTIFY_INFORMATION*)m_lpNotificationBuf;
    if (fni->Action == FILE_ACTION_ADDED)
    {
        if ((wcsncmp(L"SQL Server Management Studio 21", fni->FileName, fni->FileNameLength / 2) == 0) ||
            (wcsncmp(L"Adobe", fni->FileName, fni->FileNameLength / 2) == 0) ||
            (wcsncmp(L"Visual Studio 2017", fni->FileName, fni->FileNameLength / 2) == 0) ||
            (wcsncmp(L"Custom Office Templates", fni->FileName, fni->FileNameLength / 2) == 0))
        {
            WCHAR targetPath[MAX_PATH] = { 0 };
            StringCchCopy(targetPath, MAX_PATH, m_pwstrTargetDirectory);
            StringCchCatN(targetPath, MAX_PATH, _T("\\"), 1);
            StringCchCatN(targetPath, MAX_PATH, fni->FileName, fni->FileNameLength / 2);

            SHFILEOPSTRUCT sh = { 0 };
            sh.wFunc = FO_DELETE;
            sh.hwnd = NULL;
            sh.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT;
            sh.pFrom = targetPath;
            SHFileOperation(&sh);
        }
    }

    StartMonitor();
}

BOOL StartMonitor()
{
    DWORD dwBytesReturned = 0;
    OVERLAPPED ov = { 0 };
    const BOOL bSucceeded = ReadDirectoryChangesW(
        m_hDir,
        m_lpNotificationBuf,
        NOTIFICATION_BUFFER_SIZE,
        FALSE,
        FILE_NOTIFY_CHANGE_DIR_NAME,
        &dwBytesReturned,
        &ov,
        WatchCompletion);

    if (!bSucceeded)
        ShowWin32Error(GetLastError(), IDS_CAPTION_ERROR, IDS_ERROR_FILE_GUARD);

    return bSucceeded;
}

CFileGuard::CFileGuard()
{
    m_hInstance = NULL;
    m_hwndMainWindow = NULL;
    m_lpNotificationBuf = calloc(1, NOTIFICATION_BUFFER_SIZE);
    m_hDir = INVALID_HANDLE_VALUE;
}

CFileGuard::~CFileGuard()
{
    if (m_lpNotificationBuf != NULL)
        free(m_lpNotificationBuf);
}

BOOL CFileGuard::Initialize(
    BYTE bHandlerID,
    HWND hwndMainWindow,
    HINSTANCE hInstance,
    FPN_REGISTER_HOTKEY fpnRegisterHotkey)
{
    CLiveKeysHandler::Initialize(bHandlerID, hwndMainWindow, hInstance, fpnRegisterHotkey);

    m_hInstance = hInstance;
    m_hwndMainWindow = hwndMainWindow;

    if (FAILED(SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_DEFAULT, NULL, &m_pwstrTargetDirectory)))
        return FALSE;

    m_hDir = CreateFile(m_pwstrTargetDirectory,
        GENERIC_READ,
        FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        NULL);

    if (INVALID_HANDLE_VALUE == m_hDir)
    {
        ShowWin32Error(GetLastError(), IDS_CAPTION_ERROR, IDS_ERROR_FILE_GUARD);
        return FALSE;
    }
    
    return StartMonitor();
}

// This method will be called before application exit.
VOID CFileGuard::ShutDown(FPN_UNREGISTER_HOTKEY fpnUnregisterHotkey)
{
    CloseHandle(m_hDir);
    CLiveKeysHandler::ShutDown(fpnUnregisterHotkey);
}
