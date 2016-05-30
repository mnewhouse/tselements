/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef SERVER_STAGE_ESSENTIALS_HPP_761981345
#define SERVER_STAGE_ESSENTIALS_HPP_761981345

#include "server_stage_essentials.hpp"
#include "client_message.hpp"

#include "stage/stage_regulator.hpp"

#include "race/lap_tracker.hpp"

#include "world/world_message_fwd.hpp"

namespace ts
{
  namespace server
  {
    class MessageConveyor;
    class MessageDispatcher;

    // This class wraps the essential components that we need
    // for the server-sided part of a stage.
    class StageEssentials
    {
    public:
      explicit StageEssentials(std::unique_ptr<stage::Stage> stage_ptr,
                               const MessageDispatcher* message_dispatcher,
                               const MessageConveyor* message_conveyor);

      void update(std::uint32_t frame_duration);
      
      void handle_message(const ClientMessage<stage::messages::ControlUpdate>& update_message);
      void handle_message(const world::messages::ControlPointHit& cp_hit);

      const stage::StageDescription& stage_description() const;
      const stage::Stage* stage() const;      

    private:
      stage::StageRegulator stage_regulator_;
      race::LapTracker lap_tracker_;

      const MessageDispatcher* message_dispatcher_;
      const MessageConveyor* message_conveyor_;
    };

  }
}

#endif