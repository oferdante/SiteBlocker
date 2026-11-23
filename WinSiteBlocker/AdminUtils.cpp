#include "AdminUtils.h"

#include <windows.h>
#include <shellapi.h>

#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Advapi32.lib")


bool IsProcessElevated()
{
    HANDLE token = nullptr;

    if (!OpenProcessToken(
        GetCurrentProcess(),
        TOKEN_QUERY,
        &token))
    {
        return false;
    }

    TOKEN_ELEVATION elevation{};
    DWORD returned = 0;

    BOOL result = GetTokenInformation(
        token,
        TokenElevation,
        &elevation,
        sizeof(elevation),
        &returned
    );

    CloseHandle(token);

    return result && elevation.TokenIsElevated != 0;
}


bool RelaunchAsAdministrator()
{
    wchar_t exePath[MAX_PATH] = {};

    DWORD length = GetModuleFileNameW(
        nullptr,
        exePath,
        MAX_PATH
    );

    if (length == 0 || length >= MAX_PATH)
        return false;

    HINSTANCE result = ShellExecuteW(
        nullptr,
        L"runas",
        exePath,
        nullptr,
        nullptr,
        SW_SHOWNORMAL
    );

    return reinterpret_cast<INT_PTR>(result) > 32;
}


std::wstring GetHostsFilePath()
{
    wchar_t windowsDir[MAX_PATH] = {};

    UINT length = GetWindowsDirectoryW(
        windowsDir,
        MAX_PATH
    );

    if (length == 0 || length >= MAX_PATH)
        return L"";

    return std::wstring(windowsDir) +
        L"\\System32\\drivers\\etc\\hosts";
}