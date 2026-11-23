#include <Windows.h>

#include "MainWindow.h"
#include "HostsManager.h"
#include "DomainUtils.h"
#include "AdminUtils.h"

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <thread>

namespace
{
    HostsManager g_hostsManager(GetHostsFilePath());
    constexpr wchar_t CLASS_NAME[] = L"SiteBlocker";

    constexpr int ID_EDIT_DOMAIN = 1001;
    constexpr int ID_CHECK_WWW = 1002;
    constexpr int ID_BUTTON_ADD = 1003;
    constexpr int ID_LIST_DOMAINS = 1004;
    constexpr int ID_BUTTON_REMOVE = 1005;

    constexpr int ID_TITLE = 1100;
    constexpr int ID_SUBTITLE = 1101;
    constexpr int ID_ADD_TITLE = 1102;
    constexpr int ID_ADD_SUBTITLE = 1103;
    constexpr int ID_INPUT_LABEL = 1104;
    constexpr int ID_OPTIONS_LABEL = 1105;
    constexpr int ID_HINT = 1106;
    constexpr int ID_LIST_TITLE = 1107;
    constexpr int ID_COUNT = 1108;
    constexpr int ID_EMPTY = 1109;
    constexpr int ID_STATUS = 1110;

    constexpr COLORREF UI_COLOR_BACKGROUND = RGB(9, 11, 15);
    constexpr COLORREF COLOR_PANEL = RGB(21, 25, 33);
    constexpr COLORREF COLOR_INPUT = RGB(29, 34, 44);
    constexpr COLORREF COLOR_LIST = RGB(16, 20, 27);
    constexpr COLORREF COLOR_ACCENT = RGB(59, 130, 246);
    constexpr COLORREF COLOR_GREEN = RGB(34, 197, 94);
    constexpr COLORREF COLOR_TEXT = RGB(241, 245, 249);
    constexpr COLORREF COLOR_MUTED = RGB(148, 163, 184);
    constexpr COLORREF COLOR_SELECTED = RGB(43, 58, 79);
    constexpr COLORREF COLOR_BORDER = RGB(45, 52, 65);

    HWND g_window = nullptr;

    HWND g_title = nullptr;
    HWND g_subtitle = nullptr;

    HWND g_addTitle = nullptr;
    HWND g_addSubtitle = nullptr;
    HWND g_inputLabel = nullptr;
    HWND g_optionsLabel = nullptr;
    HWND g_hint = nullptr;

    HWND g_editDomain = nullptr;
    HWND g_checkWww = nullptr;
    HWND g_buttonAdd = nullptr;

    HWND g_listTitle = nullptr;
    HWND g_count = nullptr;
    HWND g_list = nullptr;
    HWND g_empty = nullptr;
    HWND g_buttonRemove = nullptr;

    HWND g_status = nullptr;

    HBRUSH g_backgroundBrush = nullptr;
    HBRUSH g_panelBrush = nullptr;
    HBRUSH g_inputBrush = nullptr;
    HBRUSH g_listBrush = nullptr;
    HBRUSH g_selectedBrush = nullptr;
    HBRUSH g_accentBrush = nullptr;
    HBRUSH g_greenBrush = nullptr;

    HFONT g_titleFont = nullptr;
    HFONT g_headingFont = nullptr;
    HFONT g_normalFont = nullptr;
    HFONT g_smallFont = nullptr;

    RECT g_leftPanel{};
    RECT g_rightPanel{};

    void CreateResources()
    {
        g_backgroundBrush = CreateSolidBrush(UI_COLOR_BACKGROUND);
        g_panelBrush = CreateSolidBrush(COLOR_PANEL);
        g_inputBrush = CreateSolidBrush(COLOR_INPUT);
        g_listBrush = CreateSolidBrush(COLOR_LIST);
        g_selectedBrush = CreateSolidBrush(COLOR_SELECTED);
        g_accentBrush = CreateSolidBrush(COLOR_ACCENT);
        g_greenBrush = CreateSolidBrush(COLOR_GREEN);

        g_titleFont = CreateFontW(
            -29, 0, 0, 0, FW_BOLD,
            FALSE, FALSE, FALSE,
            DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY,
            DEFAULT_PITCH,
            L"Segoe UI"
        );

        g_headingFont = CreateFontW(
            -20, 0, 0, 0, FW_SEMIBOLD,
            FALSE, FALSE, FALSE,
            DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY,
            DEFAULT_PITCH,
            L"Segoe UI"
        );

        g_normalFont = CreateFontW(
            -16, 0, 0, 0, FW_NORMAL,
            FALSE, FALSE, FALSE,
            DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY,
            DEFAULT_PITCH,
            L"Segoe UI"
        );

        g_smallFont = CreateFontW(
            -13, 0, 0, 0, FW_NORMAL,
            FALSE, FALSE, FALSE,
            DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY,
            DEFAULT_PITCH,
            L"Segoe UI"
        );
    }

    HWND CreateLabel(
        HWND parent,
        int id,
        const wchar_t* text,
        HFONT font,
        DWORD style = SS_LEFT)
    {
        HWND label = CreateWindowExW(
            0,
            L"STATIC",
            text,
            WS_CHILD | WS_VISIBLE | style,
            0, 0, 10, 10,
            parent,
            reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
            GetModuleHandleW(nullptr),
            nullptr
        );

        if (label && font)
        {
            SendMessageW(
                label,
                WM_SETFONT,
                reinterpret_cast<WPARAM>(font),
                TRUE
            );
        }

        return label;
    }

    void CreateControls(HWND hwnd)
    {
        g_title = CreateLabel(
            hwnd,
            ID_TITLE,
            L"SiteBlocker",
            g_titleFont
        );

        g_subtitle = CreateLabel(
            hwnd,
            ID_SUBTITLE,
            L"Управление блокировкой сайтов и доменов",
            g_normalFont
        );

        g_addTitle = CreateLabel(
            hwnd,
            ID_ADD_TITLE,
            L"Добавление сайта",
            g_headingFont
        );

        g_addSubtitle = CreateLabel(
            hwnd,
            ID_ADD_SUBTITLE,
            L"Введите URL или доменное имя",
            g_smallFont
        );

        g_inputLabel = CreateLabel(
            hwnd,
            ID_INPUT_LABEL,
            L"АДРЕС САЙТА",
            g_smallFont
        );

        g_editDomain = CreateWindowExW(
            WS_EX_CLIENTEDGE,
            L"EDIT",
            L"",
            WS_CHILD | WS_VISIBLE |
            WS_TABSTOP |
            ES_AUTOHSCROLL,
            0, 0, 10, 10,
            hwnd,
            reinterpret_cast<HMENU>(
                static_cast<INT_PTR>(ID_EDIT_DOMAIN)
                ),
            GetModuleHandleW(nullptr),
            nullptr
        );

        SendMessageW(
            g_editDomain,
            WM_SETFONT,
            reinterpret_cast<WPARAM>(g_normalFont),
            TRUE
        );

        g_optionsLabel = CreateLabel(
            hwnd,
            ID_OPTIONS_LABEL,
            L"ПАРАМЕТРЫ БЛОКИРОВКИ",
            g_smallFont
        );

        g_checkWww = CreateWindowExW(
            0,
            L"BUTTON",
            L"Добавлять вариант с www / без www",
            WS_CHILD | WS_VISIBLE |
            WS_TABSTOP |
            BS_AUTOCHECKBOX,
            0, 0, 10, 10,
            hwnd,
            reinterpret_cast<HMENU>(
                static_cast<INT_PTR>(ID_CHECK_WWW)
                ),
            GetModuleHandleW(nullptr),
            nullptr
        );

        SendMessageW(
            g_checkWww,
            WM_SETFONT,
            reinterpret_cast<WPARAM>(g_normalFont),
            TRUE
        );

        SendMessageW(
            g_checkWww,
            BM_SETCHECK,
            BST_CHECKED,
            0
        );

        g_hint = CreateLabel(
            hwnd,
            ID_HINT,
            L"Например, для youtube.com дополнительно "
            L"будет добавлен www.youtube.com.\n"
            L"Другие поддомены автоматически не блокируются.",
            g_smallFont
        );

        g_buttonAdd = CreateWindowExW(
            0,
            L"BUTTON",
            L"Добавить в блокировку",
            WS_CHILD | WS_VISIBLE |
            WS_TABSTOP |
            BS_OWNERDRAW,
            0, 0, 10, 10,
            hwnd,
            reinterpret_cast<HMENU>(
                static_cast<INT_PTR>(ID_BUTTON_ADD)
                ),
            GetModuleHandleW(nullptr),
            nullptr
        );

        g_listTitle = CreateLabel(
            hwnd,
            ID_LIST_TITLE,
            L"Заблокированные сайты",
            g_headingFont
        );

        g_count = CreateLabel(
            hwnd,
            ID_COUNT,
            L"0 доменов",
            g_smallFont,
            SS_RIGHT
        );

        g_list = CreateWindowExW(
            0,
            L"LISTBOX",
            nullptr,
            WS_CHILD | WS_VISIBLE |
            WS_VSCROLL |
            WS_TABSTOP |
            LBS_NOTIFY |
            LBS_OWNERDRAWFIXED |
            LBS_HASSTRINGS,
            0, 0, 10, 10,
            hwnd,
            reinterpret_cast<HMENU>(
                static_cast<INT_PTR>(ID_LIST_DOMAINS)
                ),
            GetModuleHandleW(nullptr),
            nullptr
        );

        g_empty = CreateLabel(
            hwnd,
            ID_EMPTY,
            L"Пока нет заблокированных сайтов",
            g_normalFont,
            SS_CENTER
        );

        g_buttonRemove = CreateWindowExW(
            0,
            L"BUTTON",
            L"Удалить выбранный домен",
            WS_CHILD | WS_VISIBLE |
            WS_TABSTOP |
            BS_OWNERDRAW,
            0, 0, 10, 10,
            hwnd,
            reinterpret_cast<HMENU>(
                static_cast<INT_PTR>(ID_BUTTON_REMOVE)
                ),
            GetModuleHandleW(nullptr),
            nullptr
        );

        g_status = CreateLabel(
            hwnd,
            ID_STATUS,
            L"Готово.",
            g_smallFont
        );

        EnableWindow(g_buttonRemove, FALSE);
    }

    void LayoutControls(HWND hwnd)
    {
        RECT rc{};
        GetClientRect(hwnd, &rc);

        const int width = rc.right;
        const int height = rc.bottom;

        const int margin = 24;
        const int gap = 18;

        const int panelTop = 116;
        const int panelBottom = height - 20;
        const int panelHeight = panelBottom - panelTop;

        const int availableWidth = width - margin * 2 - gap;

        const int leftWidth = std::clamp(
            static_cast<int>(availableWidth * 0.39),
            310,
            370
        );

        const int rightWidth = availableWidth - leftWidth;

        const int leftX = margin;
        const int rightX = leftX + leftWidth + gap;

        g_leftPanel = {
            leftX,
            panelTop,
            leftX + leftWidth,
            panelBottom
        };

        g_rightPanel = {
            rightX,
            panelTop,
            rightX + rightWidth,
            panelBottom
        };

        MoveWindow(g_title, margin, 21, 450, 38, TRUE);
        MoveWindow(g_subtitle, margin + 2, 61, 500, 22, TRUE);

        MoveWindow(
            g_addTitle,
            leftX + 22,
            panelTop + 20,
            leftWidth - 44,
            28,
            TRUE
        );

        MoveWindow(
            g_addSubtitle,
            leftX + 22,
            panelTop + 51,
            leftWidth - 44,
            20,
            TRUE
        );

        MoveWindow(
            g_inputLabel,
            leftX + 22,
            panelTop + 99,
            leftWidth - 44,
            20,
            TRUE
        );

        MoveWindow(
            g_editDomain,
            leftX + 22,
            panelTop + 123,
            leftWidth - 44,
            30,
            TRUE
        );

        MoveWindow(
            g_optionsLabel,
            leftX + 22,
            panelTop + 185,
            leftWidth - 44,
            20,
            TRUE
        );

        MoveWindow(
            g_checkWww,
            leftX + 22,
            panelTop + 216,
            leftWidth - 44,
            44,
            TRUE
        );

        MoveWindow(
            g_hint,
            leftX + 22,
            panelTop + 270,
            leftWidth - 44,
            50,
            TRUE
        );

        MoveWindow(
            g_buttonAdd,
            leftX + 22,
            panelBottom - 62,
            leftWidth - 44,
            42,
            TRUE
        );

        MoveWindow(
            g_listTitle,
            rightX + 22,
            panelTop + 20,
            rightWidth - 170,
            30,
            TRUE
        );

        MoveWindow(
            g_count,
            rightX + rightWidth - 155,
            panelTop + 25,
            133,
            22,
            TRUE
        );

        const int listX = rightX + 18;
        const int listY = panelTop + 66;
        const int listWidth = rightWidth - 36;
        const int listHeight = panelHeight - 144;

        MoveWindow(
            g_list,
            listX,
            listY,
            listWidth,
            listHeight,
            TRUE
        );

        MoveWindow(
            g_empty,
            listX + 10,
            listY + listHeight / 2 - 15,
            listWidth - 20,
            30,
            TRUE
        );

        MoveWindow(
            g_buttonRemove,
            rightX + 18,
            panelBottom - 62,
            rightWidth - 36,
            42,
            TRUE
        );

        MoveWindow(
            g_status,
            margin,
            height - 36,
            width - margin * 2,
            22,
            TRUE
        );

        InvalidateRect(hwnd, nullptr, TRUE);
    }

    std::vector<std::wstring> ReadBlockedDomains()
    {
        std::vector<std::wstring> domains;

        const std::wstring hostsPath = GetHostsFilePath();

        HANDLE file = CreateFileW(
            hostsPath.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ |
            FILE_SHARE_WRITE |
            FILE_SHARE_DELETE,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );

        if (file == INVALID_HANDLE_VALUE)
            return domains;

        LARGE_INTEGER fileSize{};

        if (!GetFileSizeEx(file, &fileSize) ||
            fileSize.QuadPart < 0 ||
            fileSize.QuadPart > 5 * 1024 * 1024)
        {
            CloseHandle(file);
            return domains;
        }

        std::string content(
            static_cast<size_t>(fileSize.QuadPart),
            '\0'
        );

        DWORD bytesRead = 0;

        if (!content.empty())
        {
            if (!ReadFile(
                file,
                content.data(),
                static_cast<DWORD>(content.size()),
                &bytesRead,
                nullptr))
            {
                CloseHandle(file);
                return domains;
            }

            content.resize(bytesRead);
        }

        CloseHandle(file);

        constexpr char MARKER[] = "# SiteBlocker:";

        std::istringstream stream(content);
        std::string line;

        while (std::getline(stream, line))
        {
            const size_t markerPosition = line.find(MARKER);

            if (markerPosition == std::string::npos)
                continue;

            std::string domain = line.substr(
                markerPosition + sizeof(MARKER) - 1
            );

            const size_t first = domain.find_first_not_of(" \t\r\n");

            if (first == std::string::npos)
                continue;

            const size_t last = domain.find_last_not_of(" \t\r\n");

            domain = domain.substr(first, last - first + 1);

            std::wstring wideDomain(
                domain.begin(),
                domain.end()
            );

            const bool alreadyExists = std::any_of(
                domains.begin(),
                domains.end(),
                [&wideDomain](const std::wstring& item)
                {
                    return _wcsicmp(
                        item.c_str(),
                        wideDomain.c_str()
                    ) == 0;
                }
            );

            if (!alreadyExists)
                domains.push_back(wideDomain);
        }

        std::sort(
            domains.begin(),
            domains.end(),
            [](const std::wstring& a, const std::wstring& b)
            {
                return _wcsicmp(a.c_str(), b.c_str()) < 0;
            }
        );

        return domains;
    }

    void RefreshList()
    {
        SendMessageW(g_list, LB_RESETCONTENT, 0, 0);

        const std::vector<std::wstring> domains =
            ReadBlockedDomains();

        for (const auto& domain : domains)
        {
            SendMessageW(
                g_list,
                LB_ADDSTRING,
                0,
                reinterpret_cast<LPARAM>(domain.c_str())
            );
        }

        const std::wstring countText =
            L"Всего: " + std::to_wstring(domains.size());

        SetWindowTextW(g_count, countText.c_str());

        ShowWindow(
            g_empty,
            domains.empty() ? SW_SHOW : SW_HIDE
        );

        EnableWindow(g_buttonRemove, FALSE);
    }

    void SetStatus(const std::wstring& message)
    {
        SetWindowTextW(g_status, message.c_str());
    }

    void HandleAdd()
    {
        wchar_t buffer[2048]{};

        GetWindowTextW(
            g_editDomain,
            buffer,
            static_cast<int>(_countof(buffer))
        );

        const std::wstring input = buffer;

        if (input.empty())
        {
            SetStatus(L"Введите адрес сайта.");
            SetFocus(g_editDomain);
            return;
        }

        std::wstring domain;

        if (!DomainUtils::ExtractDomain(input, domain))
        {
            SetStatus(L"Некорректный URL.");

            return;
        }

        if (!DomainUtils::IsValidDomain(domain))
        {
            SetStatus(L"Некорректный домен.");

            return;
        }

        if (domain.empty() || !DomainUtils::IsValidDomain(domain))
        {
            SetStatus(L"Некорректный URL или домен.");
            return;
        }

        const bool addWwwVariant =
            SendMessageW(
                g_checkWww,
                BM_GETCHECK,
                0,
                0
            ) == BST_CHECKED;

        std::wstring error;

        if (!g_hostsManager.AddDomain(
            domain,
            &error,
            addWwwVariant))
        {
            SetStatus(L"Не удалось добавить домен.");

            MessageBoxW(
                g_window,
                error.c_str(),
                L"Ошибка блокировки",
                MB_OK | MB_ICONERROR
            );

            return;
        }

        SetWindowTextW(g_editDomain, L"");

        RefreshList();

        SetStatus(
            L"Сайт успешно добавлен в блокировку: " + domain
        );
    }

    void HandleRemove()
    {
        const LRESULT selectedIndex = SendMessageW(
            g_list,
            LB_GETCURSEL,
            0,
            0
        );

        if (selectedIndex == LB_ERR)
        {
            SetStatus(L"Сначала выберите домен из списка.");
            return;
        }

        wchar_t buffer[512]{};

        SendMessageW(
            g_list,
            LB_GETTEXT,
            selectedIndex,
            reinterpret_cast<LPARAM>(buffer)
        );

        const std::wstring domain = buffer;

        const int answer = MessageBoxW(
            g_window,
            (L"Удалить блокировку домена " + domain + L"?").c_str(),
            L"Подтверждение",
            MB_YESNO | MB_ICONQUESTION
        );

        if (answer != IDYES)
            return;

        std::wstring error;

        if (!g_hostsManager.RemoveDomain(domain, error))
        {
            SetStatus(L"Не удалось удалить блокировку.");

            MessageBoxW(
                g_window,
                error.c_str(),
                L"Ошибка",
                MB_OK | MB_ICONERROR
            );

            return;
        }

        RefreshList();

        SetStatus(L"Блокировка удалена: " + domain);
    }

    void DrawPanel(
        HDC dc,
        const RECT& rect)
    {
        HGDIOBJ oldBrush = SelectObject(dc, g_panelBrush);
        HGDIOBJ oldPen = SelectObject(
            dc,
            GetStockObject(NULL_PEN)
        );

        RoundRect(
            dc,
            rect.left,
            rect.top,
            rect.right,
            rect.bottom,
            18,
            18
        );

        SelectObject(dc, oldBrush);
        SelectObject(dc, oldPen);
    }

    void DrawButton(
        const DRAWITEMSTRUCT* item)
    {
        const bool isAddButton =
            item->CtlID == ID_BUTTON_ADD;

        const bool disabled =
            (item->itemState & ODS_DISABLED) != 0;

        COLORREF background;

        if (isAddButton)
        {
            background = disabled
                ? RGB(71, 85, 105)
                : COLOR_ACCENT;
        }
        else
        {
            background = disabled
                ? RGB(51, 65, 85)
                : RGB(71, 85, 105);
        }

        HBRUSH brush = CreateSolidBrush(background);

        HGDIOBJ oldBrush = SelectObject(item->hDC, brush);
        HGDIOBJ oldPen = SelectObject(
            item->hDC,
            GetStockObject(NULL_PEN)
        );

        RoundRect(
            item->hDC,
            item->rcItem.left,
            item->rcItem.top,
            item->rcItem.right,
            item->rcItem.bottom,
            10,
            10
        );

        SetBkMode(item->hDC, TRANSPARENT);

        SetTextColor(
            item->hDC,
            disabled
            ? RGB(148, 163, 184)
            : RGB(255, 255, 255)
        );

        wchar_t text[128]{};

        GetWindowTextW(
            item->hwndItem,
            text,
            static_cast<int>(_countof(text))
        );

        SelectObject(item->hDC, g_normalFont);

        RECT textRect = item->rcItem;

        DrawTextW(
            item->hDC,
            text,
            -1,
            &textRect,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE
        );

        SelectObject(item->hDC, oldBrush);
        SelectObject(item->hDC, oldPen);

        DeleteObject(brush);
    }

    void DrawListItem(const DRAWITEMSTRUCT* item)
    {
        if (item->itemID == static_cast<UINT>(-1))
            return;

        const bool selected =
            (item->itemState & ODS_SELECTED) != 0;

        FillRect(
            item->hDC,
            &item->rcItem,
            selected ? g_selectedBrush : g_listBrush
        );

        wchar_t text[512]{};

        SendMessageW(
            item->hwndItem,
            LB_GETTEXT,
            item->itemID,
            reinterpret_cast<LPARAM>(text)
        );

        HGDIOBJ oldBrush = SelectObject(
            item->hDC,
            g_greenBrush
        );

        HGDIOBJ oldPen = SelectObject(
            item->hDC,
            GetStockObject(NULL_PEN)
        );

        Ellipse(
            item->hDC,
            item->rcItem.left + 15,
            item->rcItem.top + 14,
            item->rcItem.left + 27,
            item->rcItem.top + 26
        );

        SelectObject(item->hDC, oldBrush);
        SelectObject(item->hDC, oldPen);

        SetBkMode(item->hDC, TRANSPARENT);
        SetTextColor(item->hDC, COLOR_TEXT);

        SelectObject(item->hDC, g_normalFont);

        RECT textRect = item->rcItem;

        textRect.left += 40;

        DrawTextW(
            item->hDC,
            text,
            -1,
            &textRect,
            DT_LEFT | DT_VCENTER | DT_SINGLELINE
        );

        SetTextColor(item->hDC, COLOR_GREEN);

        RECT tagRect = item->rcItem;

        tagRect.right -= 15;

        DrawTextW(
            item->hDC,
            L"BLOCKED",
            -1,
            &tagRect,
            DT_RIGHT | DT_VCENTER | DT_SINGLELINE
        );
    }

    void DrawItem(const DRAWITEMSTRUCT* item)
    {
        switch (item->CtlID)
        {
        case ID_BUTTON_ADD:
        case ID_BUTTON_REMOVE:
            DrawButton(item);
            break;

        case ID_LIST_DOMAINS:
            DrawListItem(item);
            break;
        }
    }

    LRESULT CALLBACK WindowProc(
        HWND hwnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam)
    {
        switch (message)
        {
        case WM_CREATE:
        {
            g_window = hwnd;

            CreateResources();
            CreateControls(hwnd);

            LayoutControls(hwnd);
            RefreshList();

            SetStatus(L"Готово. Добавьте сайт для блокировки.");

            return 0;
        }

        case WM_SIZE:
        {
            LayoutControls(hwnd);
            return 0;
        }

        case WM_GETMINMAXINFO:
        {
            auto* info =
                reinterpret_cast<MINMAXINFO*>(lParam);

            info->ptMinTrackSize.x = 790;
            info->ptMinTrackSize.y = 590;

            return 0;
        }

        case WM_ERASEBKGND:
            return 1;

        case WM_PAINT:
        {
            PAINTSTRUCT ps{};

            HDC dc = BeginPaint(hwnd, &ps);

            RECT client{};

            GetClientRect(hwnd, &client);

            FillRect(
                dc,
                &client,
                g_backgroundBrush
            );

            DrawPanel(dc, g_leftPanel);
            DrawPanel(dc, g_rightPanel);

            HGDIOBJ oldBrush = SelectObject(
                dc,
                g_greenBrush
            );

            HGDIOBJ oldPen = SelectObject(
                dc,
                GetStockObject(NULL_PEN)
            );

            SelectObject(dc, oldBrush);
            SelectObject(dc, oldPen);

            SetBkMode(dc, TRANSPARENT);
            SetTextColor(dc, COLOR_GREEN);

            SelectObject(dc, g_smallFont);

            RECT headerStatus{
                client.right - 160,
                33,
                client.right - 25,
                57
            };

            EndPaint(hwnd, &ps);

            return 0;
        }

        case WM_CTLCOLORSTATIC:
        {
            HDC hdc = reinterpret_cast<HDC>(wParam);
            HWND control = reinterpret_cast<HWND>(lParam);

            const int id = GetDlgCtrlID(control);

            switch (id)
            {
            case ID_TITLE:
            case ID_SUBTITLE:
                SetBkColor(hdc, UI_COLOR_BACKGROUND);
                break;

            case ID_EMPTY:
                SetBkColor(hdc, COLOR_LIST);
                break;

            default:
                SetBkColor(hdc, COLOR_PANEL);
                break;
            }

            SetBkMode(hdc, OPAQUE);

            switch (id)
            {
            case ID_TITLE:
            case ID_ADD_TITLE:
            case ID_LIST_TITLE:
                SetTextColor(hdc, COLOR_TEXT);
                break;

            case ID_STATUS:
                SetTextColor(hdc, COLOR_GREEN);
                break;

            default:
                SetTextColor(hdc, COLOR_MUTED);
                break;
            }

            switch (id)
            {
            case ID_TITLE:
            case ID_SUBTITLE:
                return reinterpret_cast<LRESULT>(g_backgroundBrush);

            case ID_EMPTY:
                return reinterpret_cast<LRESULT>(g_listBrush);

            default:
                return reinterpret_cast<LRESULT>(g_panelBrush);
            }
        }

        case WM_CTLCOLOREDIT:
        {
            HDC hdc = (HDC)wParam;

            SetTextColor(hdc, COLOR_TEXT);
            SetBkColor(hdc, COLOR_INPUT);

            static HBRUSH hBrush = CreateSolidBrush(COLOR_INPUT);

            return (LRESULT)hBrush;
        }

        case WM_CTLCOLORLISTBOX:
        {
            HDC hdc = (HDC)wParam;

            SetTextColor(hdc, COLOR_TEXT);
            SetBkColor(hdc, COLOR_LIST);

            static HBRUSH hBrush = CreateSolidBrush(COLOR_LIST);

            return (LRESULT)hBrush;
        }

        case WM_CTLCOLORBTN:
        {
            HDC hdc = (HDC)wParam;

            SetTextColor(hdc, COLOR_TEXT);
            SetBkColor(hdc, COLOR_PANEL);

            static HBRUSH hBrush = CreateSolidBrush(COLOR_PANEL);

            return (LRESULT)hBrush;
        }

        case WM_MEASUREITEM:
        {
            auto* item =
                reinterpret_cast<MEASUREITEMSTRUCT*>(lParam);

            if (item->CtlID == ID_LIST_DOMAINS)
            {
                item->itemHeight = 44;
                return TRUE;
            }

            return FALSE;
        }

        case WM_DRAWITEM:
        {
            DrawItem(
                reinterpret_cast<DRAWITEMSTRUCT*>(lParam)
            );

            return TRUE;
        }

        case WM_COMMAND:
        {
            const int id = LOWORD(wParam);
            const int notification = HIWORD(wParam);

            if (id == ID_BUTTON_ADD &&
                notification == BN_CLICKED)
            {
                HandleAdd();
                return 0;
            }

            if (id == ID_BUTTON_REMOVE &&
                notification == BN_CLICKED)
            {
                HandleRemove();
                return 0;
            }

            if (id == ID_LIST_DOMAINS &&
                notification == LBN_SELCHANGE)
            {
                const LRESULT selected = SendMessageW(
                    g_list,
                    LB_GETCURSEL,
                    0,
                    0
                );

                EnableWindow(
                    g_buttonRemove,
                    selected != LB_ERR
                );
                return 0;
            }
            return 0;
        }

        case WM_DESTROY:
        {
            DeleteObject(g_backgroundBrush);
            DeleteObject(g_panelBrush);
            DeleteObject(g_inputBrush);
            DeleteObject(g_listBrush);
            DeleteObject(g_selectedBrush);
            DeleteObject(g_accentBrush);
            DeleteObject(g_greenBrush);

            DeleteObject(g_titleFont);
            DeleteObject(g_headingFont);
            DeleteObject(g_normalFont);
            DeleteObject(g_smallFont);

            PostQuitMessage(0);

            return 0;
        }
        }

        return DefWindowProcW(
            hwnd,
            message,
            wParam,
            lParam
        );
    }
}

bool RegisterMainWindowClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wc{};

    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = CLASS_NAME;

    if (RegisterClassExW(&wc))
        return true;

    return GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

HWND CreateMainWindow(HINSTANCE hInstance)
{
    if (!RegisterMainWindowClass(hInstance))
        return nullptr;

    return CreateWindowExW(
        0,
        CLASS_NAME,
        L"SiteBlocker",
        WS_OVERLAPPEDWINDOW |
        WS_CLIPCHILDREN,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1080,
        720,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );
}