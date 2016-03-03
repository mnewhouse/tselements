/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "race.hpp"

namespace ts
{
  namespace race
  {
    Race::Race(std::uint16_t lap_count, std::uint16_t control_point_count)
      : lap_tracker_(lap_count, control_point_count)
    {}

    void Race::handle_message(const world::messages::ControlPointHit& cp_hit)
    {
      lap_tracker_.control_point_hit(cp_hit.entity, cp_hit.point_id, cp_hit.frame_offset);
    }

    void Race::update_race_time(std::uint32_t frame_duration)
    {
      lap_tracker_.update_race_time(frame_duration);
    }
  }
}