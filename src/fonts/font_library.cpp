/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/


#include "font_library.hpp"

namespace ts
{
  namespace fonts
  {
    const BitmapFont& FontLibrary::load_font(const std::string& font_name, const std::string& config_path)
    {
      auto it = loaded_fonts_.find(font_name);
      if (it != loaded_fonts_.end()) return it->second;

      auto insert_result = loaded_fonts_.insert(std::make_pair(font_name, BitmapFont(config_path)));
      return insert_result.first->second;
    }

    const BitmapFont* FontLibrary::get_font_by_name(const std::string& font_name) const
    {
      auto it = loaded_fonts_.find(font_name);
      if (it == loaded_fonts_.end()) return nullptr;

      return &it->second;
    }
  }
}