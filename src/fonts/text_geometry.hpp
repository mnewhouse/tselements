/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef TEXT_GEOMETRY_HPP_289013891
#define TEXT_GEOMETRY_HPP_289013891

#include "graphics/geometry.hpp"

#include "utility/vector2.hpp"
#include "utility/color.hpp"

#include <boost/utility/string_ref.hpp>

#include <vector>
#include <cstdint>

namespace ts
{
  namespace fonts
  {
    class BitmapFont;

    namespace text_style
    {
      enum : std::uint32_t
      {
        center_horizontal = 1,
        center_vertical = 2,
        word_wrap = 4,
      };
    }

    class TextGeometry
      : private graphics::Geometry
    {
    public:
      void add_text(boost::string_ref text, const BitmapFont& font, FloatRect area, Colorb color,
                    std::uint32_t text_flags);

      using graphics::Geometry::draw;
      using graphics::Geometry::clear;

    private:
      struct CacheComponent
      {
        std::vector<vertex_type> vertices;
        const graphics::Texture* texture;
      };

      std::vector<CacheComponent> vertex_cache_;
    };
  }
}

#endif