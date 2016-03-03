/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef SERVER_HPP_38951835
#define SERVER_HPP_38951835

#include <memory>
#include <cstdint>

namespace ts
{
  namespace stage
  {
    class Stage;
    struct StageDescription;
  }

  namespace client
  {
    class MessageConveyor;
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

    // The Server class encapsulates all state and functionality that is required for a server-side cup.
    class Server
    {
    public:
      explicit Server(resources::ResourceStore* resource_store);
      ~Server();

      const stage::Stage* stage() const;
      const MessageConveyor& message_conveyor() const;

      void initiate_local_connection(const client::MessageConveyor* message_conveyor,
                                     const cup::PlayerDefinition* players, std::size_t player_count);

      void update(std::uint32_t frame_duration);

      const cup::Cup& cup() const;

    private:
      struct Impl;
      std::unique_ptr<Impl> impl_;
    };
  }
}

#endif