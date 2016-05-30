/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "lap_tracker.hpp"

#include "world/entity.hpp"
#include "world/world_limits.hpp"
#include "world/entity_id_conversion.hpp"

namespace ts
{
  namespace race
  {
    LapTracker::LapTracker(std::uint16_t lap_count, std::uint16_t control_point_count)
      : lap_count_(lap_count),
        control_point_count_(control_point_count),
        car_info_(world::limits::max_car_count),
        all_laptimes_(world::limits::max_car_count * lap_count, 0)        
    {
    }

    void LapTracker::control_point_hit(const world::Entity* car, std::uint16_t point_id,
                                       std::uint32_t frame_offset, LapEventHandler& lap_event_handler)
    {
      auto car_id = world::entity_id_to_car_id(car->entity_id());
      if (world::entity_id_to_car_id(car_id) == car->entity_id())
      {
        auto& car_info = car_info_[car_id];
        if (car_info.current_control_point == point_id)
        {
          if (point_id == 0)
          {
            // Lap completed. Calculate the lap time.
            auto lap_time = (race_time_ - car_info.last_lap_start) + frame_offset;

            all_laptimes_[car_id * lap_count_ + car_info.laps_done] = lap_time;
            ++car_info.laps_done;

            car_info.last_lap_start += lap_time;

            messages::LapComplete event;
            event.entity = car;
            event.lap_time = lap_time;
            event.race_time = race_time_;

            // Lap completed
            lap_event_handler.on_lap_complete(event);
          }

          ++car_info.current_control_point;

          if (car_info.current_control_point >= control_point_count_)
          {
            car_info.current_control_point = 0;
          }
        }
      }
    }

    void LapTracker::update_race_time(std::uint32_t frame_duration)
    {
      race_time_ += frame_duration;
    }
  }
}