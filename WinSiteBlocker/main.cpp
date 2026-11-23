#include <Windows.h>
#include "MainWindow.h"
#include "AdminUtils.h"

int WINAPI wWinMain(
    HINSTANCE hInstance,
    HINSTANCE,
    PWSTR,
    int nCmdShow)
{
    if (!IsProcessElevated())
    {
        if (RelaunchAsAdministrator())
            return 0;

        MessageBoxW(
            nullptr,
            L"Для работы программы необходимы права администратора.",
            L"SiteBlocker",
            MB_OK | MB_ICONWARNING
        );

        return 1;
    }

    HWND hwnd = CreateMainWindow(hInstance);

    if (!hwnd)
    {
        MessageBoxW(
            nullptr,
            L"Не удалось создать главное окно.",
            L"SiteBlocker",
            MB_OK | MB_ICONERROR
        );

        return 1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg{};

    while (GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return static_cast<int>(msg.wParam);
}