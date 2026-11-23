#include "DomainUtils.h"

#include <algorithm>
#include <cwctype>


namespace DomainUtils
{

    std::wstring Trim(const std::wstring& value)
    {
        const size_t first =
            value.find_first_not_of(L" \t\r\n");

        if (first == std::wstring::npos)
            return L"";

        const size_t last =
            value.find_last_not_of(L" \t\r\n");

        return value.substr(first, last - first + 1);
    }


    bool IsValidDomain(const std::wstring& domain)
    {
        if (domain.empty() || domain.size() > 253)
            return false;

        if (domain.find(L'.') == std::wstring::npos)
            return false;

        size_t labelLength = 0;

        bool previousWasHyphen = false;
        bool onlyDigitsAndDots = true;

        for (wchar_t ch : domain)
        {
            if (ch > 127)
                return false;

            if (!((ch >= L'0' && ch <= L'9') || ch == L'.'))
                onlyDigitsAndDots = false;

            if (ch == L'.')
            {
                if (labelLength == 0 || previousWasHyphen)
                    return false;

                labelLength = 0;
                previousWasHyphen = false;

                continue;
            }

            bool isLetterOrDigit =
                (ch >= L'a' && ch <= L'z') ||
                (ch >= L'A' && ch <= L'Z') ||
                (ch >= L'0' && ch <= L'9');

            if (ch == L'-')
            {
                if (labelLength == 0)
                    return false;

                previousWasHyphen = true;
            }
            else if (isLetterOrDigit)
            {
                previousWasHyphen = false;
            }
            else
            {
                return false;
            }

            ++labelLength;

            if (labelLength > 63)
                return false;
        }

        return labelLength > 0 &&
            !previousWasHyphen &&
            !onlyDigitsAndDots;
    }


    bool ExtractDomain(
        const std::wstring& rawInput,
        std::wstring& domain)
    {
        std::wstring value = Trim(rawInput);

        if (value.empty())
            return false;

        std::transform(
            value.begin(),
            value.end(),
            value.begin(),
            [](wchar_t ch)
            {
                return static_cast<wchar_t>(towlower(ch));
            }
        );

        if (value.compare(0, 7, L"http://") == 0)
        {
            value.erase(0, 7);
        }
        else if (value.compare(0, 8, L"https://") == 0)
        {
            value.erase(0, 8);
        }

        const size_t end = value.find_first_of(L"/?#");

        if (end != std::wstring::npos)
            value.erase(end);

        const size_t colon = value.find(L':');

        if (colon != std::wstring::npos)
            value.erase(colon);

        value = Trim(value);

        while (!value.empty() && value.back() == L'.')
            value.pop_back();

        if (value.empty() ||
            value.find(L'@') != std::wstring::npos)
        {
            return false;
        }

        if (!IsValidDomain(value))
            return false;

        domain = value;

        return true;
    }


    std::string ToNarrowAscii(const std::wstring& value)
    {
        return std::string(value.begin(), value.end());
    }


    std::wstring ToWideAscii(const std::string& value)
    {
        return std::wstring(value.begin(), value.end());
    }

}