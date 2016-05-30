/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef FONT_RENDERER_HPP_182598125
#define FONT_RENDERER_HPP_182598125

#include "graphics/shader.hpp"
#include "graphics/sampler.hpp"

namespace ts
{
  namespace fonts
  {
    class TextVertexBuffer;

    class FontRenderer
    {
    public:
      FontRenderer();

      void draw(const TextVertexBuffer& vertex_buffer) const;

    private:
      graphics::ShaderProgram shader_program_;
      graphics::Sampler sampler_;
    };
  }
}

#endif