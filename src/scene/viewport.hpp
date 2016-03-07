/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef VIEWPORT_HPP_189812598125
#define VIEWPORT_HPP_189812598125

#include "camera.hpp"

#include "utility/rect.hpp"

namespace ts
{
  namespace scene
  {
    // The Viewport class simply maps a portion of the screen to
    // a camera region, quite straightforward here.
    class Viewport
    {
    public:
      explicit Viewport(DoubleRect screen_rect);

      void update_camera();

      Camera& camera();
      const Camera& camera() const;

      void set_screen_rect(DoubleRect rect);
      DoubleRect screen_rect() const;

    private:
      DoubleRect screen_rect_;
      Camera camera_;      
    };
  }
}

#endif