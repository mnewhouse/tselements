/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "race_tracker.hpp"

namespace ts
{
  namespace stage
  {
    struct RaceData
    {
      explicit RaceData(std::uint16_t lap_count, std::uint16_t control_point_count);

      RaceTracker race_tracker;
    };
  }
}
