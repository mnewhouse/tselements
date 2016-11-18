/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "server_stage_essentials.hpp"
#include "server_message_distributor.hpp"

#include "stage/race_event_translator.hpp"

#include "world/world_event_translator_detail.hpp"
#include "world/world_messages.hpp"

namespace ts
{
  namespace server
  {
    template <typename MessageDispatcher, typename MessageConveyor>
    StageEssentials<MessageDispatcher, MessageConveyor>::StageEssentials(std::unique_ptr<stage::Stage> stage_ptr,
                                                                         const MessageDispatcher* message_dispatcher,
                                                                         const MessageConveyor* message_conveyor)
      : stage_regulator_(std::move(stage_ptr)),
        message_dispatcher_(message_dispatcher),
        message_conveyor_(message_conveyor)
    {}

    template <typename MessageDispatcher, typename MessageConveyor>
    void StageEssentials<MessageDispatcher, MessageConveyor>::update(std::uint32_t frame_duration)
    {
      // Create our own interception interface
      auto message_distributor = make_message_distributor(message_dispatcher_, message_conveyor_);
      auto event_interface = world::make_world_event_translator(message_distributor);

      stage_regulator_.update(frame_duration, event_interface);
    }

    template <typename MessageDispatcher, typename MessageConveyor>
    void StageEssentials<MessageDispatcher, MessageConveyor>::handle_message(const ClientMessage<client::messages::Update>& update_message)
    {
      update(update_message.message.frame_duration);
    }

    template <typename MessageDispatcher, typename MessageConveyor>
    void StageEssentials<MessageDispatcher, MessageConveyor>::handle_message(const ClientMessage<stage::messages::ControlUpdate>& update_message)
    {
      stage_regulator_.handle_message(update_message.message);
    }

    template <typename MessageDispatcher, typename MessageConveyor>
    void StageEssentials<MessageDispatcher, MessageConveyor>::handle_message(const world::messages::ControlPointHit& cp_hit)
    {
      auto message_distributor = make_message_distributor(message_dispatcher_, message_conveyor_);

      auto event_interface = stage::make_race_event_translator(message_distributor);
      stage_regulator_.control_point_hit(cp_hit, event_interface);
    }

    template <typename MessageDispatcher, typename MessageConveyor>
    const stage::StageDescription& StageEssentials<MessageDispatcher, MessageConveyor>::stage_description() const
    {
      return stage_regulator_.stage()->stage_description();
    }

    template <typename MessageDispatcher, typename MessageConveyor>
    const stage::Stage* StageEssentials<MessageDispatcher, MessageConveyor>::stage() const
    {
      return stage_regulator_.stage();
    }
  }
}
