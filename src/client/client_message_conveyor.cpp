/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "client_message_conveyor.hpp"

#include "stage/stage_messages.hpp"
#include "stage/race_messages.hpp"
#include "world/world_messages.hpp"

namespace ts
{
  namespace client
  {
    template <typename Receiver, typename MessageType>
    auto process_helper(Receiver* receiver, const MessageType& msg, int dummy) -> decltype(receiver->handle_message(msg))
    {
      if (receiver)
      {
        receiver->handle_message(msg);
      }
    }

    template <typename Receiver, typename MessageType>
    void process_helper(Receiver* receiver, const MessageType& msg, short dummy)
    {
    }

    template <typename MessageType>
    void MessageConveyor::process_internal(const MessageType& message) const
    {
      process_helper(action_state_, message, 0);
    }

    namespace
    {
      void instantiation_dummy()
      {
        namespace m = stage::messages;
        namespace w = world::messages;

        MessageConveyor c;
        c.process(m::LapComplete());
        c.process(m::SectorComplete());
        c.process(m::RaceTimeUpdate());  
        c.process(w::ControlPointHit());
        c.process(w::EntityCollision());
        c.process(w::SceneryCollision());
      }
    }
  }
}