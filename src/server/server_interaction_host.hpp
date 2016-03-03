/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef SERVER_INTERACTION_HOST_HPP_48918192839
#define SERVER_INTERACTION_HOST_HPP_48918192839

#include "server_message_dispatcher.hpp"
#include "remote_client_map.hpp"
#include "client_message.hpp"

#include <cstddef>

namespace ts
{
  namespace cup
  {
    struct PlayerDefinition;
  }

  namespace server
  {
    class CupController;
    class MessageDispatcher;

    class InteractionHost
    {
    public:
      explicit InteractionHost(CupController* cup_controller, const MessageDispatcher* message_dispatcher);

      void register_client(local_client_t, const cup::PlayerDefinition* players, 
                           std::size_t player_count);

      void handle_ready_signal(const RemoteClient& client);

    private:
      void register_client(const RemoteClient& client, const cup::PlayerDefinition* players, 
                           std::size_t player_count);

      template <typename MessageType>
      void dispatch_message(MessageType&& message, const RemoteClient& client);
      
      CupController* cup_controller_;
      const MessageDispatcher* message_dispatcher_;
      RemoteClientMap remote_client_map_;
    };
  }
}

#endif