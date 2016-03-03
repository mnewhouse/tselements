/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef RACE_HPP_331958871
#define RACE_HPP_331958871

#include "lap_tracker.hpp"

#include "world/world_messages.hpp"

#include <boost/optional.hpp>

namespace ts
{
  namespace race
  {
    class Race
    {
    public:
      explicit Race(std::uint16_t lap_count, std::uint16_t control_point_count);

      void handle_message(const world::messages::ControlPointHit& cp_hit);

      void update_race_time(std::uint32_t frame_duration);

    private:
      LapTracker lap_tracker_;
    };

    struct RaceHost : boost::optional<Race> {};
  }
}

#endif