/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "camera_3d.hpp"

#include "utility/rect.hpp"

namespace ts
{
  namespace scene3d
  {
    // The Viewport class simply maps a portion of the screen to
    // a camera region, quite straightforward here.
    class Viewport
    {
    public:
      explicit Viewport(IntRect screen_rect);

      Camera& camera();
      const Camera& camera() const;

      void set_screen_rect(IntRect rect);
      IntRect screen_rect() const;

    private:
      IntRect screen_rect_;
      Camera camera_;
    };

    glm::mat4 projection_matrix(const Viewport& view_port);
    glm::mat4 inverse_projection_matrix(const Viewport& view_port);
  }
}