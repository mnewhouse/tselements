/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "server_message_dispatcher.hpp"
#include "client_message.hpp"

#include "stage/stage_regulator.hpp"

#include "world/world_message_fwd.hpp"
#include "client/client_message_fwd.hpp"

namespace ts
{
  namespace server
  {
    class Cup;

    // This class wraps the essential components that we need
    // for the server-sided part of a stage.
    class Stage
    {
    public:
      explicit Stage(std::unique_ptr<stage::Stage> stage_ptr, Cup* cup_obj = nullptr);

      void update(std::uint32_t frame_duration);

      const stage::StageDescription& stage_description() const;
      const stage::Stage* stage() const;

      template <typename MessageType>
      void handle_message(const MessageType&) {} // Generic catch-all overload

      void handle_message(const ClientMessage<client::messages::Update>& update_request);
      void handle_message(const ClientMessage<stage::messages::ControlUpdate>& update_message);
      void handle_message(const ClientMessage<client::messages::LocalConnection>& connect_message);
      void handle_message(const ClientMessage<world::messages::CarPropertiesUpdate>& car_update);

      // Internal message (server <-> server)
      void handle_message(const world::messages::ControlPointHit& cp_hit);

      // Car editor message
      

    private:
      stage::StageRegulator stage_regulator_;
      MessageDispatcher message_dispatcher_;
    };
  }
}