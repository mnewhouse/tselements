/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/


#include "server_interaction_host.hpp"
#include "server_cup_controller.hpp"

#include "utility/random.hpp"

#include <algorithm>

namespace ts
{
  namespace server
  {
    InteractionHost::InteractionHost(CupController* cup_controller)
      : cup_controller_(cup_controller),
        remote_client_map_(cup::max_client_count)
    {
    }

    void InteractionHost::register_client(local_client_t, const cup::PlayerDefinition* players,
                                          std::size_t player_count)
    {
      register_client(RemoteClient(local_client), players, player_count);
    }

    void InteractionHost::register_client(const RemoteClient& client, const cup::PlayerDefinition* players, 
                                          std::size_t player_count)
    {
      auto result = cup_controller_->register_client(players, player_count);
      if (result.second == cup::RegistrationStatus::Success)
      {
        remote_client_map_.insert(result.first, client);

        cup::messages::RegistrationSuccess success;
        success.client_key = utility::random_integer<std::uint64_t>();
        success.client_id = static_cast<std::uint16_t>(result.first);

        dispatch_message(success, client);
      }

      else if (result.second == cup::RegistrationStatus::TooManyClients ||
               result.second == cup::RegistrationStatus::TooManyPlayers)

      {
        dispatch_message(cup::messages::ServerFull{}, local_client);
      }
    }

    void InteractionHost::handle_ready_signal(const RemoteClient& client)
    {      
      if (auto client_id = remote_client_map_.find(client))
      {
        cup_controller_->handle_ready_signal(*client_id);
      }
    }

    template <typename MessageType>
    void InteractionHost::dispatch_message(const MessageType& message, const RemoteClient& client)
    {
      cup_controller_->dispatch_message(make_client_message(message, client));
    }
  }
}