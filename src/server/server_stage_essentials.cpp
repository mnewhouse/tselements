/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "server_stage_essentials.hpp"

namespace ts
{
  namespace server
  {
    StageEssentials::StageEssentials(std::unique_ptr<stage::Stage> stage_ptr)
      : stage_regulator_(std::move(stage_ptr))
    {}

    void StageEssentials::update(world::EventInterface& event_interface, std::uint32_t frame_duration)
    {
      stage_regulator_.update(event_interface, frame_duration);
    }

    void StageEssentials::handle_message(const ClientMessage<stage::messages::ControlUpdate>& update_message)
    {
      stage_regulator_.handle_message(update_message.message);
    }

    const stage::StageDescription& StageEssentials::stage_description() const
    {
      return stage_regulator_.stage()->stage_description();
    }

    const stage::Stage* StageEssentials::stage() const
    {
      return stage_regulator_.stage();
    }
  }
}