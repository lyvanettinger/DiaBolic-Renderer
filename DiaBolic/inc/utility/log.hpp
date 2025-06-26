#pragma once

#include "spdlog/spdlog.h"

#include <codecvt>

namespace dblog = spdlog;

namespace Util
{
    inline std::string wStringToString(const std::wstring& wide)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.to_bytes(wide);
    }

    inline std::string wStringToString(const std::wstring_view& wide)
    {
        return wStringToString(std::wstring(wide));
    }

    inline std::wstring StringTowString(const std::string& str) 
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.from_bytes(str);
    }
}