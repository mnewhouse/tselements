/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef ACTION_STATE_HPP_66892213
#define ACTION_STATE_HPP_66892213

#include "client/client_action_interface.hpp"

#include "game/game_state.hpp"

#include <memory>

namespace ts
{
  namespace scene
  {
    struct Scene;
  }

  namespace controls
  {
    class ControlCenter;
  }

  namespace client
  {
    // The ActionState class template simply forwards the incoming game events
    // to the internal client state.
    template <typename MessageDispatcher>
    class ActionState
      : public game::GameState
    {
    public:
      ActionState(const game_context& game_context, ActionInterface<MessageDispatcher> action_interface);

      virtual void render(const render_context&) const override;
      virtual void update(const update_context&) override;
      virtual void process_event(const event_type&) override;

    private:
      ActionInterface<MessageDispatcher> action_interface_;
    };
  }
}

#endif