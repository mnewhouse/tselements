/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef LOCAL_CLIENT_HPP_228198531
#define LOCAL_CLIENT_HPP_228198531

#include "client_base.hpp"
#include "local_message_dispatcher.hpp"

#include "server/server.hpp"

#include <cstdint>

namespace ts
{
  namespace client
  {
    // The local client is simply a Server and a Client object tied together
    // with a local message dispatcher.
    class LocalClient
      : private server::Server, public ClientBase<LocalMessageDispatcher>
    {
    public:
      explicit LocalClient(const game::GameContext& game_context);

      void update(std::uint32_t frame_duration);
    };

    template <>
    struct ClientType<LocalMessageDispatcher>
    {
      using type = LocalClient;
    };

    template <typename MessageDispatcher>
    struct MessageContext;

    template <typename MessageDispatcher>
    class MessageConveyor;

    using LocalMessageContext = MessageContext<LocalMessageDispatcher>;
    using LocalMessageConveyor = MessageConveyor<LocalMessageDispatcher>;
  }
}

#endif