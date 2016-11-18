/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "graphics/shader.hpp"
#include "graphics/sampler.hpp"

#include <glm/mat4x4.hpp>

namespace ts
{
  namespace fonts
  {
    class TextGeometry;

    class FontRenderer
    {
    public:
      FontRenderer();

      void draw(const TextGeometry& vertex_buffer, const glm::mat4& view_matrix) const;

    private:
      graphics::ShaderProgram shader_program_;
      graphics::Sampler sampler_;
    };
  }
}
