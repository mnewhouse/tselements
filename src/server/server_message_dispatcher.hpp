/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef SERVER_MESSAGE_DISPATCHER_HPP_138912892135
#define SERVER_MESSAGE_DISPATCHER_HPP_138912892135

#include "remote_client.hpp"

#include "client/client_message_conveyor.hpp"

#include <utility>

namespace ts
{
  namespace server
  {
    class MessageDispatcher
    {
    public:
      template <typename MessageType>
      void operator()(MessageType&& message, const RemoteClient& remote_client = all_clients) const;

      void initiate_local_connection(const client::MessageConveyor* client_message_conveyor);

    private:
      const client::MessageConveyor* local_client_ = nullptr;
    };

    template <typename MessageType>
    void MessageDispatcher::operator()(MessageType&& message, const RemoteClient& remote_client) const
    {
      if (local_client_ && remote_client.type() == ClientType::Local || remote_client.type() == ClientType::All)
      {
        (*local_client_)(std::forward<MessageType>(message));
      }
    }
  }
}


#endif