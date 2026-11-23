#include "HostsManager.h"
#include "DomainUtils.h"

#include <windows.h>

#include <algorithm>
#include <vector>
#include <cwchar>
#include <cctype>
#include <cstring>
#include <sstream>
#include <set>

namespace
{
    constexpr char MARKER_PREFIX[] =
        "# SiteBlocker: ";

    std::string TrimAscii(const std::string& value)
    {
        const size_t first =
            value.find_first_not_of(" \t\r\n");

        if (first == std::string::npos)
            return "";

        const size_t last =
            value.find_last_not_of(" \t\r\n");

        return value.substr(first, last - first + 1);
    }

    std::string GetManagedDomain(const std::string& line)
    {
        const size_t position = line.find(MARKER_PREFIX);

        if (position == std::string::npos)
            return "";

        const size_t start =
            position + std::strlen(MARKER_PREFIX);

        return TrimAscii(line.substr(start));
    }

}

HostsManager::HostsManager(const std::wstring& hostsPath)
    : hostsPath_(hostsPath)
{
}

std::wstring HostsManager::GetLastErrorText(
    const wchar_t* operation)
{
    const DWORD errorCode = GetLastError();

    return std::wstring(operation) +
        L". Код ошибки Windows: " +
        std::to_wstring(errorCode);
}

bool HostsManager::ReadHostsFile(
    std::string& contents,
    std::wstring& error) const
{
    HANDLE file = CreateFileW(
        hostsPath_.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (file == INVALID_HANDLE_VALUE)
    {
        error = GetLastErrorText(
            L"Не удалось открыть файл hosts для чтения"
        );

        return false;
    }

    LARGE_INTEGER fileSize{};

    if (!GetFileSizeEx(file, &fileSize))
    {
        error = GetLastErrorText(
            L"Не удалось определить размер файла hosts"
        );

        CloseHandle(file);
        return false;
    }

    if (fileSize.QuadPart < 0 ||
        fileSize.QuadPart > 10 * 1024 * 1024)
    {
        error = L"Файл hosts слишком большой.";

        CloseHandle(file);
        return false;
    }

    contents.assign(
        static_cast<size_t>(fileSize.QuadPart), '\0'
    );

    size_t offset = 0;

    while (offset < contents.size())
    {
        const DWORD toRead = static_cast<DWORD>(
            std::min<size_t>(
                contents.size() - offset,
                1024 * 1024
            )
            );

        DWORD bytesRead = 0;

        if (!ReadFile(
            file,
            contents.data() + offset,
            toRead,
            &bytesRead,
            nullptr))
        {
            error = GetLastErrorText(
                L"Ошибка чтения файла hosts"
            );

            CloseHandle(file);
            return false;
        }

        if (bytesRead == 0)
            break;

        offset += bytesRead;
    }

    CloseHandle(file);

    contents.resize(offset);

    return true;
}

bool HostsManager::EnsureOriginalBackup(
    std::wstring& error) const
{
    const std::wstring backupPath =
        hostsPath_ + L".winblocker.bak";

    const DWORD attributes =
        GetFileAttributesW(backupPath.c_str());

    if (attributes != INVALID_FILE_ATTRIBUTES)
        return true;

    if (!CopyFileW(
        hostsPath_.c_str(),
        backupPath.c_str(),
        TRUE))
    {
        error = GetLastErrorText(
            L"Не удалось создать резервную копию hosts"
        );

        return false;
    }

    return true;
}

bool HostsManager::WriteHostsFile(
    const std::string& contents,
    std::wstring& error) const
{
    if (!EnsureOriginalBackup(error))
        return false;

    HANDLE file = CreateFileW(
        hostsPath_.c_str(),
        GENERIC_WRITE,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (file == INVALID_HANDLE_VALUE)
    {
        error = GetLastErrorText(
            L"Не удалось открыть hosts для записи"
        );

        return false;
    }

    LARGE_INTEGER beginning{};

    if (!SetFilePointerEx(
        file,
        beginning,
        nullptr,
        FILE_BEGIN))
    {
        error = GetLastErrorText(
            L"Не удалось переместить указатель файла"
        );

        CloseHandle(file);
        return false;
    }

    size_t offset = 0;

    while (offset < contents.size())
    {
        const DWORD toWrite = static_cast<DWORD>(
            std::min<size_t>(
                contents.size() - offset,
                1024 * 1024
            )
            );

        DWORD bytesWritten = 0;

        if (!WriteFile(
            file,
            contents.data() + offset,
            toWrite,
            &bytesWritten,
            nullptr) || bytesWritten == 0)
        {
            error = GetLastErrorText(
                L"Ошибка записи файла hosts"
            );

            CloseHandle(file);
            return false;
        }

        offset += bytesWritten;
    }

    if (!SetEndOfFile(file))
    {
        error = GetLastErrorText(
            L"Не удалось завершить запись файла hosts"
        );

        CloseHandle(file);
        return false;
    }

    if (!FlushFileBuffers(file))
    {
        error = GetLastErrorText(
            L"Не удалось сохранить изменения файла hosts"
        );

        CloseHandle(file);
        return false;
    }

    CloseHandle(file);

    return true;
}

bool HostsManager::HasManagedEntry(
    const std::string& contents,
    const std::string& domain)
{
    size_t start = 0;

    while (start < contents.size())
    {
        const size_t end = contents.find('\n', start);

        const size_t length =
            (end == std::string::npos)
            ? contents.size() - start
            : end - start;

        const std::string line =
            contents.substr(start, length);

        if (GetManagedDomain(line) == domain)
            return true;

        if (end == std::string::npos)
            break;

        start = end + 1;
    }

    return false;
}

bool HostsManager::IsDomainAlreadyMapped(
    const std::string& contents,
    const std::string& domain)
{
    size_t start = 0;

    while (start < contents.size())
    {
        const size_t end = contents.find('\n', start);

        const size_t length =
            (end == std::string::npos)
            ? contents.size() - start
            : end - start;

        std::string line =
            contents.substr(start, length);

        const size_t comment = line.find('#');

        if (comment != std::string::npos)
            line.erase(comment);

        std::istringstream stream(line);

        std::string ip;
        std::string host;

        if (stream >> ip)
        {
            while (stream >> host)
            {
                std::transform(
                    host.begin(),
                    host.end(),
                    host.begin(),
                    [](unsigned char ch)
                    {
                        return static_cast<char>(std::tolower(ch));
                    }
                );

                if (host == domain)
                    return true;
            }
        }

        if (end == std::string::npos)
            break;

        start = end + 1;
    }

    return false;
}

std::string HostsManager::RemoveManagedLines(
    const std::string& contents,
    const std::string& domain)
{
    std::string result;

    size_t start = 0;

    while (start < contents.size())
    {
        const size_t end = contents.find('\n', start);

        const size_t lineEnd =
            (end == std::string::npos)
            ? contents.size()
            : end + 1;

        const std::string line =
            contents.substr(start, lineEnd - start);

        if (GetManagedDomain(line) != domain)
            result += line;

        start = lineEnd;
    }

    return result;
}

void HostsManager::FlushDnsCache()
{
    HMODULE dnsApi = LoadLibraryW(L"dnsapi.dll");

    if (!dnsApi)
        return;

    using FlushFunction = BOOL(WINAPI*)();

    auto flush = reinterpret_cast<FlushFunction>(
        GetProcAddress(dnsApi, "DnsFlushResolverCache")
        );

    if (flush)
        flush();

    FreeLibrary(dnsApi);
}

bool HostsManager::GetBlockedDomains(
    std::vector<std::wstring>& domains,
    std::wstring& error) const
{
    domains.clear();

    std::string contents;

    if (!ReadHostsFile(contents, error))
        return false;

    std::set<std::string> uniqueDomains;

    size_t start = 0;

    while (start < contents.size())
    {
        const size_t end = contents.find('\n', start);

        const size_t length =
            (end == std::string::npos)
            ? contents.size() - start
            : end - start;

        const std::string line =
            contents.substr(start, length);

        const std::string domain =
            GetManagedDomain(line);

        if (!domain.empty())
            uniqueDomains.insert(domain);

        if (end == std::string::npos)
            break;

        start = end + 1;
    }

    for (const std::string& domain : uniqueDomains)
    {
        domains.push_back(
            DomainUtils::ToWideAscii(domain)
        );
    }

    return true;
}


bool HostsManager::AddDomain(
    const std::wstring& domain,
    std::wstring* error,
    bool addWwwVariant)
{
    auto setError = [&](const std::wstring& message)
        {
            if (error)
                *error = message;
        };

    if (domain.empty())
    {
        setError(L"Домен не может быть пустым.");
        return false;
    }

    std::vector<std::string> domainsToAdd;

    const std::string narrowDomain =
        DomainUtils::ToNarrowAscii(domain);

    domainsToAdd.push_back(narrowDomain);

    if (addWwwVariant)
    {
        std::string alternativeDomain;

        if (narrowDomain.rfind("www.", 0) == 0)
            alternativeDomain = narrowDomain.substr(4);
        else
            alternativeDomain = "www." + narrowDomain;

        if (alternativeDomain != narrowDomain)
            domainsToAdd.push_back(alternativeDomain);
    }

    std::string contents;

    std::wstring localError;

    if (!ReadHostsFile(contents, localError))
    {
        setError(localError);
        return false;
    }

    std::vector<std::string> newDomains;

    for (const std::string& currentDomain : domainsToAdd)
    {
        if (HasManagedEntry(contents, currentDomain))
            continue;

        if (IsDomainAlreadyMapped(contents, currentDomain))
        {
            setError(
                L"Домен уже присутствует в файле hosts: " +
                DomainUtils::ToWideAscii(currentDomain)
            );

            return false;
        }

        newDomains.push_back(currentDomain);
    }

    if (newDomains.empty())
    {
        setError(L"Домен уже заблокирован.");
        return false;
    }

    if (!contents.empty() && contents.back() != '\n')
        contents += "\r\n";

    for (const std::string& currentDomain : newDomains)
    {
        contents +=
            "0.0.0.0 " +
            currentDomain +
            " " +
            MARKER_PREFIX +
            currentDomain +
            "\r\n";
    }

    if (!WriteHostsFile(contents, localError))
    {
        setError(localError);
        return false;
    }

    FlushDnsCache();

    return true;
}

bool HostsManager::RemoveDomain(
    const std::wstring& domain,
    std::wstring& error)
{
    const std::string narrowDomain =
        DomainUtils::ToNarrowAscii(domain);

    std::vector<std::string> domains;

    domains.push_back(narrowDomain);

    if (narrowDomain.rfind("www.", 0) == 0)
    {
        domains.push_back(narrowDomain.substr(4));
    }
    else
    {
        domains.push_back("www." + narrowDomain);
    }

    std::string contents;

    if (!ReadHostsFile(contents, error))
        return false;

    std::string updated = contents;

    for (const std::string& currentDomain : domains)
    {
        updated = RemoveManagedLines(
            updated,
            currentDomain
        );
    }

    if (updated == contents)
    {
        error = L"Не удалось найти записи программы.";
        return false;
    }

    if (!WriteHostsFile(updated, error))
        return false;

    FlushDnsCache();

    return true;
}