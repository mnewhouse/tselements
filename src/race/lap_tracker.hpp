/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef LAP_TRACKER_HPP_48935819385
#define LAP_TRACKER_HPP_48935819385

#include "race_messages.hpp"

#include <vector>
#include <cstdint>

namespace ts
{
  namespace world
  {
    class Entity;
  }

  namespace race
  {
    struct LapEventHandler
    {
      virtual void on_lap_complete(const messages::LapComplete& lap_event) {};
      virtual void on_sector_complete(const messages::LapComplete& sector_event) {}
    };

    // The LapTracker class keeps track of all cars' laptimes, by processing control point events
    // and dispatching an event in case of a finished lap.
    class LapTracker
    {
    public:
      explicit LapTracker(std::uint16_t lap_count, std::uint16_t control_point_count);

      void control_point_hit(const world::Entity* entity, std::uint16_t point_id,
                             std::uint32_t frame_offset, LapEventHandler& event_handler);

      void update_race_time(std::uint32_t frame_duration);

    private:

      struct TrackedCar
      {
        std::uint16_t laps_done = 0;
        std::uint16_t best_lap = 0;
        std::uint16_t current_control_point = 1;
        std::uint32_t last_lap_start = 0;
      };

      std::uint16_t lap_count_;
      std::uint16_t control_point_count_;
      std::uint32_t race_time_ = 0;

      std::vector<TrackedCar> car_info_;
      std::vector<std::uint32_t> all_laptimes_;
    };
  }
}

#endif