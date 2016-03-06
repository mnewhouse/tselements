/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef SERVER_HPP_38951835
#define SERVER_HPP_38951835

#include <memory>
#include <cstdint>

#include "local_message_conveyor.hpp"

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
    class MessageConveyor;
    struct MessageForwarder;

    class CupEssentials;

    // The Server class encapsulates all state and functionality that is required for a server-side cup.
    class Server
    {
    public:
      explicit Server(resources::ResourceStore* resource_store);
      ~Server();

      const MessageConveyor& message_conveyor() const;
      void register_local_client(const LocalConveyor* message_conveyor,
                                 const cup::PlayerDefinition* players, std::size_t player_count);

      void update(std::uint32_t frame_duration);

    private:
      std::unique_ptr<CupEssentials> cup_essentials_;
    };
  }
}

#endif