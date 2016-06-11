/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "gui_renderer.hpp"

namespace ts
{
  namespace gui
  {
    const fonts::FontRenderer& Renderer::font_renderer() const
    {
      return font_renderer_;
    }

    const graphics::GeometryRenderer& Renderer::geometry_renderer() const
    {
      return geometry_renderer_;
    }
  }
}