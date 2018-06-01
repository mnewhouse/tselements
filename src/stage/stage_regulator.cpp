/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/


#include "stage_regulator.hpp"

#include "world/world_messages.hpp"

namespace ts
{
  namespace stage
  {
    StageRegulator::StageRegulator(std::unique_ptr<Stage> stage_ptr)
      : stage_(std::move(stage_ptr))
    {
    }

    void StageRegulator::handle_message(const messages::ControlUpdate& message)
    {
      stage_->set_controllable_state(message.controllable_id, message.controls_mask);
    }

    void StageRegulator::control_point_hit(const world::messages::ControlPointHit& cp_hit,
                                           RaceEventInterface& event_interface)
    {
      stage_->control_point_hit(cp_hit.entity, cp_hit.point_id, cp_hit.point_flags, 
                                cp_hit.frame_offset, event_interface);
    }


    const Stage* StageRegulator::stage() const
    {
      return stage_.get();
    }

    void StageRegulator::update(std::uint32_t frame_duration, world::EventInterface& event_interface)
    {
      stage_->update(frame_duration, event_interface);
    }
  }
}