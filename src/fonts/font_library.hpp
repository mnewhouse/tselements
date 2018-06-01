/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "bitmap_font.hpp"

#include <unordered_map>
#include <string>

namespace ts
{
  namespace fonts
  {
    class FontLibrary
    {
    public:
      const BitmapFont& load_font(const std::string& name, const std::string& path);
      const BitmapFont* get_font_by_name(const std::string& name) const;

    private:
      std::unordered_map<std::string, BitmapFont> loaded_fonts_;
    };
  }
}
