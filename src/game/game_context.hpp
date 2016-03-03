/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "components/generic_state_machine.hpp"

namespace ts
{
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

    template <typename StateType>
    struct GameContext
    {
      components::StateMachine<StateType>* state_machine = nullptr;
      graphics::RenderWindow* render_window = nullptr;
      game::LoadingThread* loading_thread = nullptr;
      resources::ResourceStore* resource_store = nullptr;
    };
  }
}