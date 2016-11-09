/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef TRACK_SCENE_GENERATOR_HPP
#define TRACK_SCENE_GENERATOR_HPP

#include "track_scene.hpp"

#include <vector>
#include <unordered_map>
#include <memory>

namespace ts
{
  namespace resources
  {
    class Track;
  }

  namespace scene
  {
    TrackScene generate_track_scene(const resources::Track& track);
  }
}

#endif