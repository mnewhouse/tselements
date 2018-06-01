/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <memory>
#include <cstdint>

#include "server_message_conveyor.hpp"

namespace ts
{
  namespace stage
  {
    class Stage;
  }

  namespace resources
  {
    class ResourceStore;
  }

  namespace cup
  {
    class Cup;
    struct PlayerDefinition;
  }

  namespace server
  {
    class Cup;

    // The Server class encapsulates all state and functionality that is required for a server-side cup.
    class Server
    {
    public:
      explicit Server(resources::ResourceStore* resource_store);
      ~Server();     

      const MessageConveyor& message_conveyor() const;

      void update(std::uint32_t frame_duration);

    private:
      std::unique_ptr<Cup> cup_;
    };
  }
}
