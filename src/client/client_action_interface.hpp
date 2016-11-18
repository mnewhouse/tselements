/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "game/game_state.hpp"

namespace ts
{
  namespace client
  {
    template <typename MessageDispatcher>
    class ActionEssentials;

    // This class serves as an event forwarder, which allows us to tie the action state
    // class to the object containing all the necessary logic.
    template <typename MessageDispatcher>
    class ActionInterface
    {
    public:
      explicit ActionInterface(ActionEssentials<MessageDispatcher>* action_essentials);

      void process_event(const game::Event& event);
      void render(const game::RenderContext& render_context) const;
      void update(std::uint32_t frame_duration);
      
    private:
      ActionEssentials<MessageDispatcher>* action_essentials_;
    };
  }
}
