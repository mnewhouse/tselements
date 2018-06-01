/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <cstdint>

#include "client_message_conveyor.hpp"

namespace ts
{
  namespace cup
  {
    struct PlayerDefinition;
  }

  namespace client
  {
    namespace messages
    {
      // Message type that's used internally to request an update in other logical components.
      struct Update
      {
        std::uint32_t frame_duration;
      };

      struct LocalConnection
      {
        MessageConveyor message_conveyor;

        const cup::PlayerDefinition* players;
        std::size_t player_count;
      };
    }
  }
}
