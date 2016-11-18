/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "graphics/texture.hpp"

#include "utility/vector2.hpp"

#include <string>
#include <cstdint>
#include <vector>
#include <unordered_map>

namespace ts
{
  namespace fonts
  {
    struct GlyphInfo
    {
      Vector2f position;
      Vector2f size;
      Vector2f offset;
      float x_advance;
      std::uint8_t page;
      std::uint8_t channel;
    };

    namespace detail
    {
      struct BitmapFontData
      {
        std::uint32_t line_height_;
        std::uint32_t base_;
        Vector2u scale_;
        std::uint32_t page_count_;
        std::unordered_map<std::uint32_t, GlyphInfo> glyphs_;
        std::unordered_map<std::uint64_t, std::int32_t> kerning_;

        std::vector<graphics::Texture> pages_;
      };
    }

    class BitmapFont
      : private detail::BitmapFontData
    {
    public:
      explicit BitmapFont(const std::string& config_file);

      const GlyphInfo* glyph_info(std::uint32_t code_point) const;
      std::int32_t kerning(std::uint32_t first_cp, std::uint32_t second_cp) const;
      std::uint32_t line_height() const;

      const graphics::Texture& page_texture(std::size_t index) const;

    private:
    };
  }
}
