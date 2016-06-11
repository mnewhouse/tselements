/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef COLOR_HPP_49821
#define COLOR_HPP_49821

#include <cstdint>

namespace ts
{
  template <typename T>
  struct Color
  {
    Color() = default;

    Color(T r, T g, T b, T a)
      : r(r), g(g), b(b), a(a)
    {}

    T r = {};
    T g = {};
    T b = {};
    T a = {};
  };

  using Colorb = Color<std::uint8_t>;
  using Colorf = Color<float>;

  inline std::uint32_t to_integer(const Colorb& color)
  {
    std::uint32_t result = color.a;
    result |= static_cast<std::uint32_t>(color.b) << 8;
    result |= static_cast<std::uint32_t>(color.g) << 16;
    result |= static_cast<std::uint32_t>(color.r) << 24;

    return result;
  }

  inline Colorb to_color(std::uint32_t color_int)
  {
    std::uint8_t r = (color_int >> 24) & 0xFF;
    std::uint8_t g = (color_int >> 16) & 0xFF;
    std::uint8_t b = (color_int >> 8) & 0xFF;
    std::uint8_t a = color_int & 0xFF;

    return Colorb(r, g, b, a);
  }
}

#endif
