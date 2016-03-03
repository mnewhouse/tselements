/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef SERVER_MESSAGES_HPP_68963412
#define SERVER_MESSAGES_HPP_68963412

#include "remote_client.hpp"

#include <type_traits>

namespace ts
{
  namespace server
  {
    template <typename MessageType>
    struct ClientMessage
    {
      explicit ClientMessage(const MessageType& message_, RemoteClient client_)
        : message(message_), client(client_)
      {}

      const MessageType& message;
      RemoteClient client;
    };

    template <typename MessageType>
    ClientMessage<MessageType> make_client_message(const MessageType& message, RemoteClient client)
    {
      return ClientMessage<MessageType>(message, std::move(client));
    }

    template <typename MessageType>
    std::enable_if_t<!std::is_lvalue_reference<MessageType>::value>
      make_client_message(MessageType&& message, RemoteClient client) = delete;
  }
}

#endif