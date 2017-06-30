/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/


#include "race.hpp"

namespace ts
{
  namespace stage
  {
    RaceData::RaceData(std::uint16_t lap_count, std::uint16_t control_point_count)
      : race_tracker(lap_count, control_point_count)
    {}
  }
}