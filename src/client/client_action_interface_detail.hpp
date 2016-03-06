/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "client_action_interface.hpp"
#include "client_action_essentials.hpp"

namespace ts
{
  namespace client
  {
    template <typename MessageDispatcher>
    ActionInterface<MessageDispatcher>::ActionInterface(ActionEssentials<MessageDispatcher>* action_essentials)
      : action_essentials_(action_essentials)
    {
    }

    template <typename MessageDispatcher>
    void ActionInterface<MessageDispatcher>::process_event(const game::Event& event)
    {
      action_essentials_->process_event(event);
    }

    template <typename MessageDispatcher>
    void ActionInterface<MessageDispatcher>::update(std::uint32_t frame_duration)
    {
      action_essentials_->update(frame_duration);
    }

    template <typename MessageDispatcher>
    void ActionInterface<MessageDispatcher>::render(const game::RenderContext& render_context) const
    {
      action_essentials_->render(render_context);
    }
  }
}