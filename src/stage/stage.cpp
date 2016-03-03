/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stage.hpp"
#include "stage_creation.hpp"

#include "world/car.hpp"

namespace ts
{
  namespace stage
  {
    Stage::Stage(world::World world_obj, StageDescription stage_description)
      : world_(std::move(world_obj)),
        stage_description_(std::move(stage_description))
    {
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

    void Stage::update(world::EventInterface& event_interface, std::uint32_t frame_duration)
    {
      world_.update(event_interface, frame_duration);
      stage_time_ += frame_duration;
    }

    std::uint32_t Stage::stage_time() const
    {
      return stage_time_;
    }

    void Stage::set_controllable_state(std::uint16_t controllable_id, std::uint16_t controls_mask)
    {
      // TODO: if controllable id represents a car
      if (auto car = world_.find_car(static_cast<std::uint8_t>(controllable_id)))
      {
        car->update_controls_mask(controls_mask);
      }
    }
  }
}