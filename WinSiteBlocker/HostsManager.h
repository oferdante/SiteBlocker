#pragma once

#include <string>
#include <vector>

class HostsManager
{
public:

    explicit HostsManager(const std::wstring& hostsPath);

    bool GetBlockedDomains(
        std::vector<std::wstring>& domains,
        std::wstring& error
    ) const;

    bool AddDomain(
        const std::wstring& domain,
        std::wstring* error = nullptr,
        bool addWwwVariant = true
    );

    bool RemoveDomain(
        const std::wstring& domain,
        std::wstring& error
    );


private:

    bool ReadHostsFile(
        std::string& contents,
        std::wstring& error
    ) const;

    bool WriteHostsFile(
        const std::string& contents,
        std::wstring& error
    ) const;

    bool EnsureOriginalBackup(
        std::wstring& error
    ) const;

    static bool HasManagedEntry(
        const std::string& contents,
        const std::string& domain
    );

    static bool IsDomainAlreadyMapped(
        const std::string& contents,
        const std::string& domain
    );

    static std::string RemoveManagedLines(
        const std::string& contents,
        const std::string& domain
    );

    static void FlushDnsCache();

    static std::wstring GetLastErrorText(
        const wchar_t* operation
    );

private:

    std::wstring hostsPath_;
};