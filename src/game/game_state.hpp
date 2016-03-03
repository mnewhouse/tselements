/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef GAME_STATE_HPP_183952
#define GAME_STATE_HPP_183952

#include "game_context.hpp"
#include "game_events.hpp"

#include "components/generic_state.hpp"
#include "components/generic_state_machine.hpp"

#include "utility/vector2.hpp"

namespace ts
{
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
      Vector2u screen_size;
    };
    
    struct StateTraits
    {
      using game_context = GameContext<GameState>;
      using event_type = Event;

      using render_context = RenderContext;
      using update_context = UpdateContext;
    };

    class GameState
      : public components::GenericState<StateTraits>
    {
    public:
      using GenericState::GenericState;
    };
  }
}

#endif