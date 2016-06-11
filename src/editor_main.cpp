/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "graphics/render_window.hpp"
#include "graphics/gl_context.hpp"

#include "game/game_state.hpp"
#include "editor/editor_test_state.hpp"

#include <string>
#include <thread>
#include <iostream>
#include <exception>

using namespace ts;

int main(int argc, char* argv[])
{
  try
  {
    int screen_width = 1280, screen_height = 800;
    graphics::RenderWindow window("TS Elements", screen_width, screen_height, graphics::WindowMode::Windowed);
    graphics::initialize_glew();

    window.clear();
    window.display();
    window.activate();

    game::StateMachine state_machine;

    game::GameContext game_context{};
    game_context.render_window = &window;
    game_context.state_machine = &state_machine;

    state_machine.create_state<editor::TestState>(game_context);

    while (state_machine)
    {
      auto state_transition_guard = state_machine.transition_guard();

      for (sf::Event event; window.poll_event(event);)
      {
        if (event.type == sf::Event::Closed)
        {
          // Clearing the state machine is equivalent to quitting the program.
          state_machine.clear();
        }

        state_machine->process_event(event);
      }

      game::RenderContext render_context;
      render_context.frame_progress = 0.0;
      render_context.screen_size = window.size();

      window.clear();
      state_machine->render(render_context);
      window.display();

      // Hack
      // std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
  }

  catch (const std::exception& e)
  {
    std::cerr << "An unhandled except occurred: " << e.what() << std::endl;
  }
  

}