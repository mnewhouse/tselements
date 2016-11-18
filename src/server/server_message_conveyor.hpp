/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "server_message_forwarder.hpp"

#include "messages/message_conveyor.hpp"

namespace ts
{
  namespace server
  {
    class CupEssentials;

    struct MessageContext
    {
      CupEssentials* cup_essentials;
    };

    class MessageConveyor
      : public messages::MessageConveyor<MessageContext>
    {
      using messages::MessageConveyor<MessageContext>::MessageConveyor;
    };

    // The following overloads are internal messages that we use to communicate between different
    // server components.
    template <typename MessageType>
    void forward_message(const MessageContext& context, MessageType&& message)
    {
      MessageForwarder forwarder = { context.cup_essentials };
      forwarder.forward(std::forward<MessageType>(message));
    }
  }
}
