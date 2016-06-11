/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "components/generic_state_machine.hpp"

#ifndef GAME_CONTEXT_HPP_129038518923
#define GAME_CONTEXT_HPP_129038518923

namespace ts
{
  namespace gui
  {
    class Context;
  }

  namespace graphics
  {
    class RenderWindow;
  }

  namespace resources
  {
    class ResourceStore;
  }

  namespace game
  {
    class LoadingThread;

    class GameState;
    struct GameContext
    {
      components::StateMachine<GameState>* state_machine = nullptr;
      gui::Context* gui_context = nullptr;
      graphics::RenderWindow* render_window = nullptr;
      game::LoadingThread* loading_thread = nullptr;
      resources::ResourceStore* resource_store = nullptr;
    };
  }
}

#endif