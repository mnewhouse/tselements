/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef SERVER_MESSAGE_DISPATCHER_HPP_138912892135
#define SERVER_MESSAGE_DISPATCHER_HPP_138912892135

#include "remote_client.hpp"
#include "local_message_conveyor.hpp"

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

      void initiate_local_connection(const LocalConveyor* local_message_conveyor);

    private:
      const LocalConveyor* local_conveyor_ = nullptr;
    };

    template <typename MessageType>
    void MessageDispatcher::operator()(MessageType&& message, const RemoteClient& remote_client) const
    {
      if (local_conveyor_ && (remote_client.type() == ClientType::Local || remote_client.type() == ClientType::All))
      {
        (*local_conveyor_)(std::forward<MessageType>(message));
      }
    }
  }
}


#endif