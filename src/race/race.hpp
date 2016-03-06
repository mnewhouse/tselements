/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef RACE_HPP_331958871
#define RACE_HPP_331958871

#include "lap_tracker.hpp"

namespace ts
{
  namespace race
  {
    struct Race
    {
      explicit Race(std::uint16_t lap_count, std::uint16_t control_point_count);

      LapTracker lap_tracker;
    };
  }
}

#endif