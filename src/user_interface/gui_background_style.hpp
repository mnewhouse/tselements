/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef GUI_BACKGROUND_STYLE_HPP_84912384
#define GUI_BACKGROUND_STYLE_HPP_84912384

#include "graphics/texture.hpp"

#include "utility/rect.hpp"
#include "utility/color.hpp"

namespace ts
{
  namespace gui
  {
    namespace styles
    {
      struct FillArea
      {
        Colorb color;
        const graphics::Texture* texture;
        IntRect texture_rect;
      };

      auto fill_area(Colorb color)
      {
        return FillArea{ color, nullptr, IntRect() };
      }

      auto fill_area(const graphics::Texture* texture, IntRect texture_rect,
                     Colorb color = Colorb(255, 255, 255, 255))
      {
        return FillArea{ color, texture, texture_rect };
      }

      template <typename Area, typename Geometry>
      void add_background(const Area& area, const FillArea& fill_area, Geometry& geometry)
      {
        if (!fill_area.texture)
        {
          add_vertices(area, fill_area.color, geometry);
        }

        else
        {
          add_vertices(area, fill_area.color, fill_area.texture, fill_area.texture_rect, geometry);
        }        
      }
    }
  }
}

#endif