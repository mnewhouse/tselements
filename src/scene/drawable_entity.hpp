/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef DRAWABLE_ENTITY_HPP_85912859
#define DRAWABLE_ENTITY_HPP_85912859

#include "graphics/texture.hpp"

#include "world/entity.hpp"

#include "utility/color.hpp"
#include "utility/rect.hpp"
#include "utility/rotation.hpp"

#include <glm/mat4x4.hpp>
#include <glm/mat2x2.hpp>

namespace ts
{
  namespace scene
  {
    struct DrawableEntity
    {
      const world::Entity* entity = nullptr;
      const graphics::Texture* texture = nullptr;

      FloatRect texture_coords;
      FloatRect colorizer_coords;
      FloatRect frame_bounds;

      Vector2f frame_offset;
      float hover_distance = 0.0;

      glm::mat4 transformation;
      glm::mat2 colorizer_transformation;      

      Colorb colors[3];
    };
  }
}

#endif