/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CLIENT_MESSAGE_FORWARDER_HPP_242418581915
#define CLIENT_MESSAGE_FORWARDER_HPP_242418581915

#include "cup/cup_message_fwd.hpp"
#include "stage/stage_message_fwd.hpp"
#include "world/world_message_fwd.hpp"
#include "race/race_messages.hpp"

namespace ts
{
  namespace client
  {
    template <typename MessageDispatcher>
    class CupEssentials;

    // This class template defines a fixed set of message forwarding overloads to pass
    // to the essential client cup logic.
    template <typename MessageDispatcher>
    class MessageForwarder
    {
    public:
      explicit MessageForwarder(CupEssentials<MessageDispatcher>* cup_essentials_)
        : cup_essentials(cup_essentials_)
      {
      }

      void forward(const cup::messages::RegistrationSuccess&);
      void forward(const stage::messages::StageLoaded&);
      void forward(const cup::messages::StageBegin&);
      void forward(const cup::messages::StageEnd&);

      void forward(const world::messages::SceneryCollision&);
      void forward(const world::messages::EntityCollision&);

      // Have a fallback implementation for unlisted messages that simply does nothing.
      template <typename MessageType>
      void forward(const MessageType&) {}

    private:
      CupEssentials<MessageDispatcher>* cup_essentials;
    };
  }
}

#endif