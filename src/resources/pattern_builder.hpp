/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "pattern.hpp"

namespace ts
{
  namespace resources
  {
    class Track;
    class PatternLoader;

    PatternLoader preload_pattern_files(const resources::Track& track);

    Pattern build_track_pattern(const Track& track);
    Pattern build_track_pattern(const Track& track, PatternLoader& pre_loaded_patterns);   
  }
}
