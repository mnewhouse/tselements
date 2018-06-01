/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "components/generic_state_machine.hpp"

namespace ts
{
  namespace imgui
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
      imgui::Context* gui_context = nullptr;
      graphics::RenderWindow* render_window = nullptr;
      game::LoadingThread* loading_thread = nullptr;
      resources::ResourceStore* resource_store = nullptr;
    };
  }
}
