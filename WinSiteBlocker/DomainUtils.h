#pragma once

#include <string>

namespace DomainUtils
{
    std::wstring Trim(const std::wstring& value);

    bool IsValidDomain(const std::wstring& domain);

    bool ExtractDomain(
        const std::wstring& rawInput,
        std::wstring& domain
    );

    std::string ToNarrowAscii(const std::wstring& value);

    std::wstring ToWideAscii(const std::string& value);
}