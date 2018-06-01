/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "game/game_context.hpp"
#include "game/game_events.hpp"

#include <memory>

namespace ts
{
  namespace client
  {
    class MessageConveyor;

    class LocalPlayerRoster;

    // The Client class wraps the client cup logic, not really doing all that much
    // other than hiding the implementation details. The MessageDispatcher parameter
    // dictates where the outgoing messages are sent to and how that's done, allowing us
    // to use this template for both local and remote games.
    class ClientBase
    {
    public:
      ClientBase(const game::GameContext& context);      

      void process_event(const game::Event& event);

    protected:
      ~ClientBase();
      void update(std::uint32_t frame_duration);

      const LocalPlayerRoster& local_players() const;
      MessageConveyor message_conveyor() const;

    private:
      std::unique_ptr<Cup> cup_;
    };
  }
}
