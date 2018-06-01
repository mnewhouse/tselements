/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "client/client_base.hpp"

#include "game/game_state.hpp"

namespace ts
{
  namespace cup
  {
    class Cup;
  }

  namespace client
  {
    // The CupState class template holds a Client<> object and everything that encompasses.
    // It does no more than forwarding the events and calls to said object.
    class CupState
      : public game::GameState
    {
    public:
      // Args: the arguments that the Client<MessageDispatcher> member object is constructed with.
      // If omitted, client is constructed with a game context.
      template <typename... Args>
      explicit CupState(const game_context& context, Args&&... client_args)
        : game::GameState(context),
          client_(std::forward<Args>(args)...)
      {
      }

      explicit CupState(const game_context& context);      

      virtual void process_event(const event_type& event) override;
      virtual void update(const update_context&) override;

    private:
      Client client_;
    };
  }
}
