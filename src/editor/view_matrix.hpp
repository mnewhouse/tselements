/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "utility/vector2.hpp"

#include <glm/mat4x4.hpp>

namespace ts
{
  namespace scene
  {
    class Viewport;

    glm::mat4 compute_view_matrix(const Viewport& viewport, Vector2i world_size, double frame_progress = 0.0);
  }
}