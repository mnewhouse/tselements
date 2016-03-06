/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "action_state.hpp"


namespace ts
{
  namespace client
  {
    template <typename MessageDispatcher>
    ActionState<MessageDispatcher>::ActionState(const game_context& context,
                                                ActionInterface<MessageDispatcher> action_interface)
      : game::GameState(context),
        action_interface_(std::move(action_interface))
    {
    }

    template <typename MessageDispatcher>
    void ActionState<MessageDispatcher>::render(const render_context& render_ctx) const
    {
      action_interface_.render(render_ctx);
    }

    template <typename MessageDispatcher>
    void ActionState<MessageDispatcher>::update(const update_context& context)
    {
      action_interface_.update(context.frame_duration);
    }

    template <typename MessageDispatcher>
    void ActionState<MessageDispatcher>::process_event(const event_type& event)
    {
      action_interface_.process_event(event);
    }
  }
}