/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "shader.hpp"
#include "sampler.hpp"

#include <glm/mat4x4.hpp>

namespace ts
{
  namespace graphics
  {
    class Geometry;

    class GeometryRenderer
    {
    public:
      GeometryRenderer();

      void draw(const Geometry& geometry, const glm::mat4x4& view_matrix) const;

    private:
      graphics::ShaderProgram shader_program_;
      graphics::Sampler sampler_;
    };
  }
}
