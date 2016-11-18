/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "race_event_interface.hpp"

#include <vector>
#include <cstdint>
#include <string>

#include <boost/container/small_vector.hpp>

namespace ts
{
  namespace world
  {
    class Entity;
  }

  namespace stage
  {
    // The LapTracker class keeps track of all cars' laptimes, by processing control point events
    // and dispatching an event in case of a finished lap.
    class RaceTracker
    {
    public:
      explicit RaceTracker(std::uint16_t lap_count, std::uint16_t control_point_count);

      void control_point_hit(const world::Entity* entity, std::uint16_t point_id, std::uint32_t point_flags,
                             std::uint32_t frame_offset, RaceEventInterface& event_interface);

      void advance_race_time(std::uint32_t frame_duration);

      void reset_race_time(std::uint32_t new_time);

      void register_lap(const world::Entity* entity, std::uint32_t lap_time);

      struct TrackedCar
      {
        std::uint16_t laps_done = 0;
        std::uint16_t best_lap_id = 0;
        std::uint16_t current_control_point = 1;
        std::uint32_t current_sector = 0;
        std::uint32_t last_lap_start = 0;
        std::uint32_t last_lap_time = 0;
        std::uint32_t best_lap_time = 0;
        std::uint32_t last_sector_time = 0;

        boost::container::small_vector<std::uint32_t, 8> best_lap_sector_times;
        boost::container::small_vector<std::uint32_t, 8> current_lap_sector_times;
      };

      const TrackedCar* car_info(const world::Entity* car) const;
      std::uint16_t lap_count() const;
      std::uint32_t race_time() const;

    private:
      std::uint16_t lap_count_;
      std::uint16_t control_point_count_;
      std::uint32_t race_time_ = 0;

      std::vector<TrackedCar> car_info_;
      std::vector<std::uint32_t> all_laptimes_;
    };

    std::string format_lap_time(std::uint32_t lap_time);
  }
}
