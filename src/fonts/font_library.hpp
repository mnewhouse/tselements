/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef FONT_LIBRARY_HPP_4123986692
#define FONT_LIBRARY_HPP_4123986692

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

#endif