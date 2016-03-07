/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stage_regulator.hpp"

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

    const Stage* StageRegulator::stage() const
    {
      return stage_.get();
    }

    void StageRegulator::update(world::EventInterface& event_interface, std::uint32_t frame_duration)
    {
      stage_->update(event_interface, frame_duration);
    }
  }
}