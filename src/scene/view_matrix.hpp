/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <SFML/Graphics/Transform.hpp>

#include "utility/vector2.hpp"

namespace ts
{
  namespace scene
  {
    class Viewport;

    sf::Transform compute_view_matrix(const Viewport& viewport, Vector2i world_size, double frame_progress = 0.0);
  }
}
