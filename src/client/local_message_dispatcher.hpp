/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "server/server_message_conveyor.hpp"
#include "server/client_message.hpp"

#include <utility>

namespace ts
{
  namespace server
  {
    class MessageConveyor;
  }

  namespace client
  {
    // The local message dispatcher can take a message structure type and simply forwards it to 
    // the server's message conveyor, where it is relayed to the various message handling
    // classes, or ignored if the message type is not supported.
    class LocalMessageDispatcher
    {
    public:
      explicit LocalMessageDispatcher(const server::MessageConveyor* message_conveyor);

      template <typename MessageType>
      void operator()(const MessageType& message) const
      {
        (*message_conveyor_)(server::make_client_message(message, server::local_client));
      }

    private:
      const server::MessageConveyor* message_conveyor_;
    };
  }
}
