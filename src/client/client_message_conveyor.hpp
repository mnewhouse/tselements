/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CLIENT_MESSAGE_CONVEYOR_HPP_668912385
#define CLIENT_MESSAGE_CONVEYOR_HPP_668912385

#include "messages/message_conveyor.hpp"

#include "client_message_forwarder.hpp"

namespace ts
{
  namespace client
  {
    template <typename MessageDispatcher>
    class CupEssentials;

    template <typename MessageDispatcher>
    struct MessageContext
    {
      CupEssentials<MessageDispatcher>* cup_essentials;
    };

    // The MessageConveyor class is responsible for forwarding messages to the
    // client's logical components. It does this by calling forward_message defined below,
    // which uses the MessageForwarder class template.
    template <typename MessageDispatcher>
    class MessageConveyor
      : public ts::messages::MessageConveyor<MessageContext<MessageDispatcher>>
    {
      using ts::messages::MessageConveyor<MessageContext<MessageDispatcher>>::MessageConveyor;
    };
  }

  namespace client
  {
    template <typename MessageDispatcher, typename MessageType>
    void forward_message(const MessageContext<MessageDispatcher>& context, const MessageType& message)
    {
      MessageForwarder<MessageDispatcher> forwarder(context.cup_essentials);
      forwarder.forward(message);
    }
  }
}

#endif