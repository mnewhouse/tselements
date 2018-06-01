/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <string>

namespace ts
{
  namespace fonts
  {
    struct BuiltinFont
    {
      const char* path;
      const char* name;
    };

    static const BuiltinFont builtin_fonts[] =
    {
      { "fonts/default_font_18px.fnt", "default18" },
      { "fonts/extra_bold_32px.fnt", "extrabold32" }
    };

    static const std::string default_18 = builtin_fonts[0].name;
    static const std::string extrabold_32 = builtin_fonts[1].name;
  }
}
