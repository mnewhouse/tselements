/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "remote_client.hpp"
#include "server_message_conveyor.hpp"

#include "client/client_message_conveyor.hpp"

#include <utility>

namespace ts
{
  namespace server
  {
    class MessageDispatcher
    {
    public:
      explicit MessageDispatcher(MessageConveyor conveyor)
        : server_(conveyor)
      {
      }

      template <typename MessageType>
      void send(const MessageType& message, const RemoteClient& remote_client = all_clients) const
      {
        if (remote_client.type() == ClientType::Local || remote_client.type() == ClientType::All)
        {
          local_client_.process(message);
        }

        if (remote_client.type() == ClientType::All)
        {
          server_.process(message);
        }
      }

      void connect(client::MessageConveyor local_client)
      {
        local_client_ = local_client;
      }

    private:
      client::MessageConveyor local_client_;
      MessageConveyor server_;
    };
  }
}
