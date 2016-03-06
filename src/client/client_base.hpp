/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CLIENT_HPP_894531234
#define CLIENT_HPP_894531234

#include <memory>

#include "game/game_context.hpp"
#include "game/game_events.hpp"

namespace ts
{
  namespace client
  {
    template <typename MessageDispatcher>
    struct ClientType
    {
      using type = void;
    };

    // A little helper traits structure with an alias template so that we can write 
    // Client_t<MessageDispatcher>;
    template <typename MessageDispatcher>
    using Client = typename ClientType<MessageDispatcher>::type;

    template <typename MessageDispatcher>
    class CupEssentials;

    class LocalPlayerRoster;

    template <typename MessageDispatcher>
    class MessageConveyor;

    // The ClientBase class wraps the client cup logic, not really doing all that much
    // other than hiding the implementation details. The MessageDispatcher parameter
    // dictates where the outgoing messages are sent to and how that's done, allowing us
    // to use this template for both local and remote games.
    template <typename MessageDispatcher>
    class ClientBase
    {
    public:
      ClientBase(const game::GameContext& context, MessageDispatcher message_dispatcher);      

      void process_event(const game::Event& event);

    protected:
      ~ClientBase();
      void update(std::uint32_t frame_duration);

      const LocalPlayerRoster& local_players() const;
      const MessageConveyor<MessageDispatcher>& message_conveyor() const;

    private:
      std::unique_ptr<CupEssentials<MessageDispatcher>> cup_essentials_;
    };
  }
}

#endif