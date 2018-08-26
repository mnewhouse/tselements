/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/


#include "race_tracker.hpp"
#include "race_messages.hpp"

#include "resources/control_point.hpp"

#include "world/entity.hpp"
#include "world/world_limits.hpp"
#include "world/entity_id_conversion.hpp"

namespace ts
{
  namespace stage
  {
    std::string format_lap_time(std::uint32_t lap_time)
    {
      std::string result(24, 0);

      auto minutes = lap_time / 60000;
      auto seconds = (lap_time % 60000) / 1000;
      auto milliseconds = lap_time % 1000;

      if (minutes != 0)
      {
        result.resize(std::sprintf(&result[0], "%d:%02d.%03d", minutes, seconds, milliseconds));
      }

      else
      {
        result.resize(std::sprintf(&result[0], "%d.%03d", seconds, milliseconds));
      }      

      return result;
    }

    RaceTracker::RaceTracker(std::uint16_t lap_count, std::uint16_t control_point_count)
      : lap_count_(lap_count),
        control_point_count_(control_point_count),
        car_info_(world::limits::max_car_count),
        all_laptimes_(world::limits::max_car_count * lap_count, 0)        
    {
    }

    void RaceTracker::control_point_hit(const world::Entity* car, std::uint16_t point_id, std::uint32_t point_flags,
                                       std::uint32_t frame_offset, RaceEventInterface& event_interface)
    {
      auto car_id = world::entity_id_to_car_id(car->entity_id());
      if (world::entity_id_to_car_id(car_id) == car->entity_id())
      {
        auto& car_info = car_info_[car_id];
        if (car_info.current_control_point == point_id)
        {
          auto lap_time = (race_time_ - car_info.last_lap_start) + frame_offset;

          if (point_id == 0)
          {
            if (car_info.laps_done == 0 || lap_time < car_info.best_lap_time)
            {
              car_info.last_best_lap_time = car_info.best_lap_time;
              car_info.best_lap_id = car_info.laps_done;
              car_info.best_lap_time = lap_time;
              car_info.best_lap_sector_times = car_info.current_lap_sector_times;
            }

            // Lap completed. Calculate the lap time.
            all_laptimes_[car_id * lap_count_ + car_info.laps_done] = lap_time;
            ++car_info.laps_done;

            car_info.last_lap_start += lap_time;
            car_info.last_lap_time = lap_time;
            car_info.current_lap_sector_times.clear();
            car_info.current_sector = 0;

            messages::LapComplete event;
            event.entity = car;
            event.lap_time = lap_time;
            event.race_time = race_time_;

            // Lap completed
            event_interface.on_lap_complete(event);

            printf("L%d: %s\n", car_info.laps_done, format_lap_time(lap_time).c_str());        
          }

          if (point_flags & resources::ControlPoint::Sector)
          {
            if (car_info.current_sector == 0)
            {
              printf("S1: %s\n", format_lap_time(lap_time).c_str());
            }

            else
            {
              auto sector_time = lap_time - car_info.last_sector_time;
              printf("S%d: %s (%s)\n", car_info.current_sector + 1, 
                     format_lap_time(lap_time).c_str(), format_lap_time(sector_time).c_str());
            }

            car_info.last_sector_time = lap_time;
            car_info.current_lap_sector_times.push_back(lap_time);

            ++car_info.current_sector;
          }

          if (point_flags & resources::ControlPoint::SpeedTest)
          {
            printf("(%.1f km/h)\n", magnitude(car->velocity()));
          }

          ++car_info.current_control_point;

          if (car_info.current_control_point >= control_point_count_)
          {
            car_info.current_control_point = 0;
          }
        }
      }
    }

    void RaceTracker::register_lap(const world::Entity* car, std::uint32_t lap_time)
    {
      auto car_id = world::entity_id_to_car_id(car->entity_id());
      if (world::entity_id_to_car_id(car_id) == car->entity_id())
      {
        auto& car_info = car_info_[car_id];

        if (car_info.laps_done == 0 || lap_time < car_info.best_lap_time)
        {
          car_info.best_lap_id = car_info.laps_done;
          car_info.best_lap_time = lap_time;
          car_info.best_lap_sector_times = car_info.current_lap_sector_times;
        }

        all_laptimes_[car_id * lap_count_ + car_info.laps_done] = lap_time;
        ++car_info.laps_done;
        car_info.current_sector = 0;
        car_info.current_control_point = 0;
        car_info.last_sector_time = lap_time;
        car_info.last_lap_time = lap_time;
        car_info.last_lap_start += lap_time;        
      }
    }

    const RaceTracker::TrackedCar* RaceTracker::car_info(const world::Entity* car) const
    {
      auto car_id = world::entity_id_to_car_id(car->entity_id());
      if (car_id == car->entity_id())
      {
        return &car_info_[car_id];
      }

      return nullptr;
    }

    std::uint16_t RaceTracker::lap_count() const
    {
      return lap_count_;
    }

    std::uint32_t RaceTracker::race_time() const
    {
      return race_time_;
    }

    void RaceTracker::advance_race_time(std::uint32_t frame_duration)
    {
      race_time_ += frame_duration;
    }

    void RaceTracker::reset_race_time(std::uint32_t race_time)
    {
      race_time_ = race_time;
    }
  }
}