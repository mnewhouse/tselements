/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "graphics/texture.hpp"

#include "world/entity.hpp"

#include "utility/color.hpp"
#include "utility/rect.hpp"
#include "utility/rotation.hpp"

#include <array>

#include <SFML/Graphics/Transform.hpp>

namespace ts
{
  namespace scene
  {
    struct DrawableEntity
    {
      const world::Entity* entity = nullptr;
      const graphics::Texture* texture = nullptr;
      const graphics::Texture* colorizer_texture = nullptr;

      sf::Transform model_transform;
      sf::Transform new_model_transform;
      sf::Transform colorizer_transform;
      std::uint32_t level = 0;

      Vector2f texture_coords_offset;
      Vector2f texture_coords_scale;

      float shadow_offset = 0.0f;
      float new_shadow_offset = 0.0f;

      std::array<float, 9> colors = {};
    };
  }
}
