/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "graphics/render_window.hpp"
#include "graphics/gl_context.hpp"

#include "game/game_state.hpp"
#include "editor/track_edit_state.hpp"

#include "resources/resource_store.hpp"

#include "fonts/builtin_fonts.hpp"
#include "fonts/font_library.hpp"

#include "user_interface/gui_context.hpp"

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
    graphics::RenderWindow window("Project \"Free Like Bird\" - Editor",
                                  screen_width, screen_height, graphics::WindowMode::Windowed);
    window.set_framerate_limit(0);

    graphics::initialize_glew();

    window.activate();
    window.clear();
    window.display();

    resources::ResourceStore resource_store;
    for (const auto& font : fonts::builtin_fonts)
    {
      resource_store.font_library().load_font(font.name, font.path);
    }

    game::StateMachine state_machine;
    gui::Context gui_context;

    game::GameContext game_context{};
    game_context.render_window = &window;
    game_context.state_machine = &state_machine;
    game_context.gui_context = &gui_context;
    game_context.resource_store = &resource_store;

    game::UpdateContext update_context{};
    update_context.frame_duration = 20U;

    state_machine.create_state<editor::track::EditorState>(game_context);

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

      state_machine->update(update_context);

      game::RenderContext render_context;
      render_context.frame_progress = 0.0;
      render_context.screen_size = window.size();

      window.clear();
      state_machine->render(render_context);
      window.display();      
    }
  }

  catch (const std::exception& e)
  {
    std::cerr << "An unhandled except occurred: " << e.what() << std::endl;
  }
}