/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <string>
#include <codecvt>

namespace ts
{
  // The function convert_to_platform_encoding converts an UTF-8 string to
  // the platform-specific encoding. In practice this means it's converted to
  // a std::wstring on Windows, and on other systems it's a no-op.

#ifdef WIN32
  inline std::wstring convert_to_platform_encoding(const std::string& string)
  {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.from_bytes(string);
  }

#else
  inline const std::string& convert_to_platform_encoding(const std::string& string)
  {
    return string;
  }

  inline std::string convert_to_platform_encoding(std::string&& string)
  {
    return std::move(string);
  }

#endif

}
