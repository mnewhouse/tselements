/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "game_context.hpp"
#include "game_events.hpp"

#include "components/generic_state.hpp"
#include "components/generic_state_machine.hpp"

#include "utility/vector2.hpp"

namespace ts
{
  namespace gui
  {
    class Renderer;
  }

  namespace game
  {
    class GameState;
    using StateMachine = components::StateMachine<GameState>;

    struct UpdateContext
    {
      std::uint32_t frame_duration;
    };

    struct RenderContext
    {
      double frame_progress;
      Vector2i screen_size;
    };
    
    struct StateTraits
    {
      using game_context = GameContext;
      using event_type = Event;

      using render_context = RenderContext;
      using update_context = UpdateContext;
    };

    // We can just derive from the generic GameState object based on the traits class
    // we just defined. We could also use an alias, but deriving allows us to forward
    // declare this class.
    class GameState
      : public components::GenericState<StateTraits>
    {
    public:
      using GenericState::GenericState;
    };

    using components::activate;
    using components::deactivate;
  }
}
