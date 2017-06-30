/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "viewport.hpp"

namespace ts
{
  namespace scene
  {
    Viewport::Viewport(IntRect screen_rect)
      : screen_rect_(screen_rect)
    {
    }

    void Viewport::set_screen_rect(IntRect rect)
    {
      screen_rect_ = rect;
    }

    IntRect Viewport::screen_rect() const
    {
      return screen_rect_;
    }

    const Camera& Viewport::camera() const
    {
      return camera_;
    }

    Camera& Viewport::camera()
    {
      return camera_;
    }

    void Viewport::update_camera()
    {
      return camera_.update_position();
    }
  }
}