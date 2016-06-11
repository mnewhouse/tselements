/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef GUI_GEOMETRY_HPP_5819238512
#define GUI_GEOMETRY_HPP_5819238512

#include "gui_text_style.hpp"
#include "graphics/geometry.hpp"
#include "fonts/text_geometry.hpp"

#include "utility/color.hpp"

#include <boost/utility/string_ref.hpp>

namespace ts
{
  namespace gui
  {
    class Renderer;
    struct RenderState;

    struct Geometry
    {
      graphics::Geometry geometry;
      fonts::TextGeometry text;
    };

    template <typename Style>
    void add_text(boost::string_ref text, FloatRect area,
                  const Style& text_style, Geometry& geometry)
    
    {
      std::uint32_t text_flags = 0;
      if (center_horizontal(text_style)) text_flags |= fonts::text_style::center_horizontal;
      if (center_vertical(text_style)) text_flags |= fonts::text_style::center_vertical;

      geometry.text.add_text(text, font(text_style), area, text_color(text_style), text_flags);
    }


    void add_vertices(FloatRect area, Colorb color, Geometry& geometry);


    void add_vertices(FloatRect area, Colorb color, const graphics::Texture* texture,
                      IntRect texture_rect, Geometry& geometry);

    void draw(const Renderer& renderer, const Geometry& geometry, const RenderState& render_state);
  }
}

#endif