/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "server_stage_essentials.hpp"
#include "server_message_dispatcher.hpp"
#include "server_message_conveyor.hpp"
#include "client_message.hpp"

#include "stage/stage_regulator.hpp"

#include "world/world_message_fwd.hpp"

#include "client/update_message.hpp"

namespace ts
{
  namespace server
  {
    // This class wraps the essential components that we need
    // for the server-sided part of a stage.
    template <typename MessageDispatcher, typename MessageConveyor>
    class StageEssentials
    {
    public:
      explicit StageEssentials(std::unique_ptr<stage::Stage> stage_ptr,
                               const MessageDispatcher* message_dispatcher,
                               const MessageConveyor* message_conveyor);

      void update(std::uint32_t frame_duration);
      
      void handle_message(const ClientMessage<client::messages::Update>& update_request);
      void handle_message(const ClientMessage<stage::messages::ControlUpdate>& update_message);

      // Internal message (server <-> server)
      void handle_message(const world::messages::ControlPointHit& cp_hit);

      const stage::StageDescription& stage_description() const;
      const stage::Stage* stage() const;      

    private:
      stage::StageRegulator stage_regulator_;

      const MessageDispatcher* message_dispatcher_;
      const MessageConveyor* message_conveyor_;
    };

    using DefaultStageEssentials = StageEssentials<MessageDispatcher, MessageConveyor>;
  }
}
