/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

/*
#include "server.hpp"
#include "server_cup.hpp"

namespace ts
{
  namespace server
  {
    Server::Server(resources::ResourceStore* resource_store)
      : cup_(std::make_unique<Cup>(resource_store))
    {
    }

    Server::~Server()
    {
    }

    void Server::update(std::uint32_t frame_duration)
    {
      cup_->update(frame_duration);
    }

    const MessageConveyor& Server::message_conveyor() const
    {
      return cup_->message_conveyor();
    }
  }
}
*/