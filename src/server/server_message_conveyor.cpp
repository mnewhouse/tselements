/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "server_message_conveyor.hpp"

#include "server_stage.hpp"

#include "client/client_messages.hpp"
#include "stage/stage_messages.hpp"
#include "stage/race_messages.hpp"
#include "world/world_messages.hpp"

namespace ts
{
  namespace server
  {
    template <typename Receiver, typename MessageType>
    auto process_helper(Receiver* receiver, const MessageType& msg, int) -> decltype(receiver->handle_message(msg))
    {
      if (receiver)
      {
        receiver->handle_message(msg);
      }
    }

    template <typename Receiver, typename MessageType>
    void process_helper(Receiver* receiver, const MessageType& msg, short)
    {
    }

    template <typename MessageType>
    void MessageConveyor::process_internal(const MessageType& message) const
    {
      process_helper(cup_, message, 0);
      process_helper(stage_, message, 0);
    }

    namespace
    {
      void instantiation_dummy()
      {
        auto cm = [](const auto& msg)
        {
          return make_client_message(msg, all_clients);
        };

        MessageConveyor c(nullptr, nullptr);
        c.process(cm(stage::messages::ControlUpdate{}));
        c.process(cm(client::messages::Update{}));
        c.process(cm(client::messages::LocalConnection{}));

        c.process(stage::messages::RaceTimeUpdate{});        
        c.process(stage::messages::LapComplete{});           
        c.process(stage::messages::SectorComplete{});

        c.process(world::messages::SceneryCollision{});
        c.process(world::messages::EntityCollision{});
        c.process(world::messages::ControlPointHit{});        
      }
    }
  }
}