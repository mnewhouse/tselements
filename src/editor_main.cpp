/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "core/config_definitions.hpp"

#include "graphics/render_window.hpp"
#include "graphics/gl_context.hpp"

#include "game/game_state.hpp"
#include "game/loading_thread.hpp"

#include "utility/debug_log.hpp"

#include "editor/track_editor_state.hpp"

#include "resources/resource_store.hpp"
#include "resources/car_store.hpp"
#include "resources/settings.hpp"
#include "cup/cup_settings.hpp"
#include "client/player_settings.hpp"


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
  debug::DebugConfig debug_config;
  debug_config.debug_level = debug::level::auxiliary;
  debug::ScopedLogger debug_log(debug_config, "editor_debug.txt");

  try
  {
    int screen_width = 1280, screen_height = 800;
    graphics::RenderWindow window("Project \"Free Like Bird\" - Editor",
                                  screen_width, screen_height, graphics::WindowMode::Windowed);
    window.set_framerate_limit(120);

    graphics::initialize_glew();

    window.activate();
    window.clear();
    window.display();

    resources::ResourceStore resource_store;
    for (const auto& font : fonts::builtin_fonts)
    {
      resource_store.font_library().load_font(font.name, font.path);
    }

    resource_store.car_store().load_car_directory("cars");

    game::StateMachine state_machine;
    game::LoadingThread loading_thread;
    gui::Context gui_context;

    game::GameContext game_context{};
    game_context.render_window = &window;
    game_context.state_machine = &state_machine;
    game_context.loading_thread = &loading_thread;
    game_context.gui_context = &gui_context;
    game_context.resource_store = &resource_store;

    auto& player_settings = resource_store.settings().player_settings();
    auto& cup_settings = resource_store.settings().cup_settings();

    cup::PlayerDefinition player;
    player.control_slot = 0;
    player.id = 0;
    player.name = "test";
    player_settings.selected_players.push_back(player);

    auto car_it = resource_store.car_store().car_definitions().find("porge");
    cup_settings.selected_cars.push_back(*car_it);

    game::UpdateContext update_context{};
    update_context.frame_duration = 20U;

    if (argc >= 2)
    {
      state_machine.create_state<editor::track::EditorState>(game_context, argv[1]);
    }

    else
    {
      state_machine.create_state<editor::track::EditorState>(game_context);
    }    

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
    std::cerr << "An unhandled exception occurred: " << e.what() << std::endl;
  }
}