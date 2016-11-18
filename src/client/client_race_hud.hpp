/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <cstdint>
#include <string>

namespace ts
{
  namespace stage
  {
    class RaceTracker;
  }

  namespace scene
  {
    class ViewportArrangement;
  }

  namespace client
  {
    class RaceHUD
    {
    public:
      void update(const scene::ViewportArrangement& viewport_arrangement, const stage::RaceTracker& race_tracker);

    private:
      std::string format_buffer_;
    };
  }
}
