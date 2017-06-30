/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/


#include "stage.hpp"
#include "stage_creation.hpp"

#include "controls/control.hpp"

#include "world/car.hpp"

namespace ts
{
  namespace stage
  {
    Stage::Stage(world::World world_obj, StageDescription stage_description)
      : world_(std::move(world_obj)),
        stage_description_(std::move(stage_description)),
        race_tracker_(100, static_cast<std::uint16_t>(world_.track().control_points().size()))
    {
      create_stage_entities();
    }

    void Stage::create_stage_entities()
    {
      stage::create_stage_entities(world_, stage_description_);
    }

    const StageDescription& Stage::stage_description() const
    {
      return stage_description_;
    }

    const world::World& Stage::world() const
    {
      return world_;
    }

    const resources::Track& Stage::track() const
    {
      return world_.track();
    }

    void Stage::update(std::uint32_t frame_duration, world::EventInterface& event_interface)
    {
      world_.update(frame_duration, event_interface);

      race_tracker_.advance_race_time(frame_duration);
      stage_time_ += frame_duration;
    }

    std::uint32_t Stage::stage_time() const
    {
      return stage_time_;
    }

    const RaceTracker* Stage::race_tracker() const
    {
      return &race_tracker_;
    }

    void Stage::control_point_hit(const world::Entity* entity, std::uint16_t point_id, std::uint32_t point_flags,
                                  std::uint32_t frame_offset, RaceEventInterface& event_interface)
    {
      race_tracker_.control_point_hit(entity, point_id, point_flags, frame_offset, event_interface);
    }

    void Stage::set_controllable_state(std::uint16_t controllable_id, controls::ControlsMask mask)
    {
      // TODO: if controllable id represents a car
      if (auto car = world_.find_car(static_cast<std::uint8_t>(controllable_id)))
      {
        car->update_controls_mask(mask);
      }
    }
  }
}