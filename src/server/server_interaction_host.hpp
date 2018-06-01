/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

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

    class InteractionHost
    {
    public:
      explicit InteractionHost(CupController* cup_controller);

      void register_client(local_client_t, const cup::PlayerDefinition* players, 
                           std::size_t player_count);

      void handle_ready_signal(const RemoteClient& client);

    private:
      void register_client(const RemoteClient& client, const cup::PlayerDefinition* players, 
                           std::size_t player_count);

      template <typename MessageType>
      void dispatch_message(const MessageType& message, const RemoteClient& client);
      
      CupController* cup_controller_;
      RemoteClientMap remote_client_map_;
    };
  }
}
