/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CLIENT_ACTION_MESSAGE_FORWARDER_HPP_21389127357
#define CLIENT_ACTION_MESSAGE_FORWARDER_HPP_21389127357

#include "world/world_message_fwd.hpp"

namespace ts
{
  namespace client
  {
    template <typename MessageDispatcher>
    class ActionEssentials;

    template <typename MessageDispatcher>
    class ActionMessageForwarder
    {
    public:
      ActionMessageForwarder(ActionEssentials<MessageDispatcher>* action_essentials_)
        : action_essentials(action_essentials_)
      {
      }

      void forward(const world::messages::SceneryCollision& collision);
      void forward(const world::messages::EntityCollision& collision);

      // Have a fallback implementation for unlisted messages that simply does nothing.
      template <typename MessageType>
      void forward(const MessageType&) {}

    private:
      ActionEssentials<MessageDispatcher>* action_essentials;
    };
  }
}

#endif