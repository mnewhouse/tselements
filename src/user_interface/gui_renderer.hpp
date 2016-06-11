/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef GUI_RENDERER_HPP_95091245819
#define GUI_RENDERER_HPP_95091245819

#include "graphics/geometry_renderer.hpp"

#include "fonts/font_renderer.hpp"

#include "utility/vector2.hpp"

namespace ts
{
  namespace gui
  {
    struct RenderState
    {
      Vector2f screen_size;
      Vector2f translation;
    };

    class Renderer
    {
    public:
      const fonts::FontRenderer& font_renderer() const;
      const graphics::GeometryRenderer& geometry_renderer() const;

    private:
      fonts::FontRenderer font_renderer_;
      graphics::GeometryRenderer geometry_renderer_;
    };
  }
}

#endif