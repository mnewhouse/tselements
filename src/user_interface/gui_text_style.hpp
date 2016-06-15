/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef GUI_TEXT_STYLE_HPP_8129481234
#define GUI_TEXT_STYLE_HPP_8129481234

#include "utility/color.hpp"
#include "utility/vector2.hpp"

#include <type_traits>
#include <cstdint>

namespace ts
{
  namespace fonts
  {
    class BitmapFont;
  }

  namespace gui
  {
    namespace styles
    {
      struct TextStyle
      {
        const fonts::BitmapFont& bitmap_font;
        Colorb text_color;
        Vector2f text_offset;
        std::uint32_t text_flags;        
      };

      struct center_text_horizontal_t : std::integral_constant<std::uint32_t, 1> {};
      struct center_text_vertical_t : std::integral_constant<std::uint32_t, 2> {};

      namespace detail
      {
        inline std::uint32_t make_text_flag_mask()
        {
          return 0;
        }

        template <typename FlagType>
        std::uint32_t make_text_flag_mask(FlagType flag)
        {
          return FlagType::value;
        }

        template <typename FlagType, typename... Flags>
        std::uint32_t make_text_flag_mask(FlagType flag, Flags... flags)
        {
          return make_text_flag_mask(flag) | make_text_flag_mask(flags...);
        }
      }     
      
      static constexpr center_text_horizontal_t center_text_horizontal;
      static constexpr center_text_vertical_t center_text_vertical;
      
      template <typename... Flags>
      auto text_style(const fonts::BitmapFont& font, Colorb color, 
                      Vector2f text_offset, Flags... flags)
      {
        return TextStyle{ font, color, text_offset, detail::make_text_flag_mask(flags...) };
      }

      inline const auto& font(const TextStyle& style)
      {
        return style.bitmap_font;
      }

      inline auto text_color(const TextStyle& style)
      {
        return style.text_color;
      }

      inline auto text_offset(const TextStyle& style)
      {
        return style.text_offset;
      }

      inline bool center_horizontal(const TextStyle& style)
      {
        return (style.text_flags & center_text_horizontal_t::value) != 0;
      }

      inline bool center_vertical(const TextStyle& style)
      {
        return (style.text_flags & center_text_vertical_t::value) != 0;
      }
    }
  }
}

#endif
