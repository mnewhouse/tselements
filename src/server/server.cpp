/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "server.hpp"
#include "server_cup_essentials.hpp"

namespace ts
{
  namespace server
  {
    Server::Server(resources::ResourceStore* resource_store)
      : cup_essentials_(std::make_unique<CupEssentials>(resource_store))
    {
    }

    Server::~Server()
    {
    }

    void Server::update(std::uint32_t frame_duration)
    {
      cup_essentials_->update(frame_duration);
    }

    void Server::register_local_client(const LocalConveyor* local_conveyor,
                                       const cup::PlayerDefinition* players, std::size_t player_count)
    {
      cup_essentials_->register_local_client(local_conveyor, players, player_count);
    }

    const MessageConveyor& Server::message_conveyor() const
    {
      return cup_essentials_->message_conveyor();
    }
  }
}