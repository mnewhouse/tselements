/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "graphics/texture.hpp"

#include "world/entity.hpp"

#include "utility/color.hpp"
#include "utility/rect.hpp"
#include "utility/rotation.hpp"

#include <glm/mat4x4.hpp>
#include <glm/mat2x2.hpp>
#include <glm/vec3.hpp>

#include <array>

namespace ts
{
  namespace scene
  {
    struct DrawableEntity
    {
      struct Vertex
      {
        glm::vec2 position;
        glm::vec2 texture_coords;
        glm::vec3 colorizer_coords;
        Colorb color;
      };

      const world::Entity* entity = nullptr;
      const graphics::Texture* texture = nullptr;

      std::uint32_t z_level = 0;

      std::array<Colorf, 3> colors = {};
      std::array<Vertex, 4> vertices;

      glm::mat4 model_matrix;
      glm::mat4 new_model_matrix;
      glm::mat4 colorizer_matrix;

      float shadow_offset = 0.0f;
      float new_shadow_offset = 0.0f;
    };
  }
}
