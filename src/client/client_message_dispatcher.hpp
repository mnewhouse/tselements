/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "stage/stage_message_fwd.hpp"

#include "server/client_message.hpp"
#include "server/server_message_conveyor.hpp"

#pragma once

namespace ts
{
  namespace client
  {
    class MessageDispatcher
    {
    public:
      explicit MessageDispatcher(server::MessageConveyor local_connection = {})
        : local_connection_(local_connection)
      {}

      template <typename MessageType>
      void send(const MessageType& message) const
      {
        local_connection_.process(server::make_client_message(message, server::local_client));      
      }

      void connect(server::MessageConveyor local_connection)
      {
        local_connection_ = local_connection;
      }

    private:
      server::MessageConveyor local_connection_;
    };
  }
}