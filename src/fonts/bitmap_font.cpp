/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "bitmap_font.hpp"

#include "graphics/image.hpp"

#include "utility/stream_utilities.hpp"

#include <boost/filesystem/path.hpp>

#include <algorithm>

namespace ts
{
  namespace fonts
  {
    namespace detail
    {
      auto make_kerning_key(std::uint32_t first, std::uint32_t second)
      {
        return static_cast<std::uint64_t>(first) << 32 | static_cast<std::uint64_t>(second);
      }

      BitmapFontData load_bitmap_font(const std::string& file_name)
      {
        auto configuration = make_ifstream(file_name, std::ios::in | std::ios::binary);
        if (!configuration)
          throw std::runtime_error("failed to load font configuration file '" + file_name + "'");

        auto contents = read_stream_contents(configuration);
        auto check = [&](bool b)
        {
          if (!b)
          {
            throw std::runtime_error("invalid font configuration file '" + file_name + "'");
          }
        };

        check(contents.size() >= 4);
        check(std::equal(contents.begin(), contents.begin() + 3, "BMF"));

        std::size_t idx = 4;

        auto byte = [&](auto idx) { return static_cast<std::uint8_t>(contents[idx]); };
        auto read32 = [&]()
        {
          check(contents.size() >= idx + 4);

          auto result = static_cast<std::uint32_t>(byte(idx++));
          result |= static_cast<std::uint32_t>(byte(idx++)) << 8;
          result |= static_cast<std::uint32_t>(byte(idx++)) << 16;
          result |= static_cast<std::uint32_t>(byte(idx++)) << 24;
          return result;
        };

        auto read16 = [&]()
        {
          check(contents.size() >= idx + 2);

          auto result = static_cast<std::uint16_t>(byte(idx++));
          result |= static_cast<std::uint16_t>(byte(idx++)) << 8;
          return result;
        };

        auto read8 = [&]() -> std::uint8_t
        {
          return byte(idx++);
        };

        auto read_string = [&]()
        {
          check(idx < contents.size());

          auto pred = [](char ch) { return ch == 0; };
          auto begin = contents.begin() + idx;
          auto end = std::find_if(begin, contents.end(), pred);

          std::string result(begin, end);
          idx += result.size() + 1;

          return result;
        };

        auto negate16 = [](std::uint16_t v) -> std::int16_t
        {
          return -static_cast<std::int16_t>(~v) - 1;
        };

        // Just skip first block, we won't need any of that
        {
          check(idx < contents.size() && contents[idx++] == 1);
  
          auto block_size = read32();
          idx += block_size;          
        }
        
        BitmapFontData result;
        std::vector<std::string> pages;

        {
          check(idx < contents.size());
          check(contents[idx++] == 2);
          auto block_size = read32();

          result.line_height_ = read16();
          result.base_ = read16();
          result.scale_.x = read16();
          result.scale_.y = read16();
          result.page_count_ = read16();

          idx += 5;         
        }

        {
          check(idx < contents.size() && contents[idx++] == 3);

          auto block_size = read32();
          for (std::uint32_t page = 0; page != result.page_count_; ++page)
          {
            check(idx < contents.size());
            pages.push_back(read_string());
          }
        }

        {
          check(idx < contents.size() && contents[idx++] == 4);

          auto block_size = read32();
          auto glyph_count = block_size / 20;

          for (std::uint32_t glyph_index = 0; glyph_index != glyph_count; ++glyph_index)
          {
            auto code_point = read32();

            GlyphInfo glyph_info;
            glyph_info.position.x = static_cast<float>(read16());
            glyph_info.position.y = static_cast<float>(read16());
            glyph_info.size.x = static_cast<float>(read16());
            glyph_info.size.y = static_cast<float>(read16());

            glyph_info.offset.x = static_cast<float>(negate16(read16()));
            glyph_info.offset.y = static_cast<float>(negate16(read16()));
            glyph_info.x_advance = static_cast<float>(negate16(read16()));
            glyph_info.page = read8();
            glyph_info.channel = read8();

            check(glyph_info.page == 0);

            result.glyphs_.insert(std::make_pair(code_point, glyph_info));         
          }

          if (idx < contents.size() && contents[idx++] == 5)
          {
            auto block_size = read32();
            auto kerning_pairs = block_size / 10;

            for (std::uint32_t pair_index = 0; pair_index != kerning_pairs; ++pair_index)
            {
              auto first = read32();
              auto second = read32();
              auto kerning = negate16(read16());

              auto key = make_kerning_key(first, second);
              result.kerning_.insert(std::make_pair(key, kerning));
            }
          }
        }

        auto directory_name = boost::filesystem::path(file_name).remove_filename();
        for (const auto& page : pages)
        {
          auto file_name = (directory_name / page).string();
          auto image = graphics::load_image(file_name.data(), file_name.size());
          result.pages_.push_back(graphics::create_texture(image));
        }        
        
        return result;
      }
    }

    BitmapFont::BitmapFont(const std::string& file_name)
      : detail::BitmapFontData(detail::load_bitmap_font(file_name))
    {
    }

    const GlyphInfo* BitmapFont::glyph_info(std::uint32_t code_point) const
    {
      auto it = glyphs_.find(code_point);
      if (it != glyphs_.end())
      {
        return &it->second;
      }

      return nullptr;
    }

    std::int32_t BitmapFont::kerning(std::uint32_t first_cp, std::uint32_t second_cp) const
    {
      auto key = detail::make_kerning_key(first_cp, second_cp);
      auto it = kerning_.find(key);
      if (it != kerning_.end())
      {
        return it->second;
      }

      return 0;
    }

    const graphics::Texture& BitmapFont::page_texture(std::size_t index) const
    {
      return pages_[index]; 
    }

    std::uint32_t BitmapFont::line_height() const
    {
      return line_height_;
    }
  }
}