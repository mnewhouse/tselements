/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "core/config_definitions.hpp"

#include "graphics/render_window.hpp"
#include "graphics/gl_context.hpp"

#include "imgui/imgui_sfml_opengl.hpp"
#include "imgui/imgui_default_style.hpp"

#include "game/main_loop.hpp"
#include "game/game_state.hpp"
#include "game/process_priority.hpp"
#include "game/loading_thread.hpp"

#include "utility/debug_log.hpp"

#include "editor/editor_state.hpp"

#include "resources/resource_store.hpp"
#include "resources/car_store.hpp"
#include "resources/settings.hpp"

#include "cup/cup_settings.hpp"
#include "client/player_settings.hpp"

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
    game::elevate_process_priority();

    int screen_width = 1280, screen_height = 800;
    graphics::RenderWindow window("Pocket Wheels - Editor",
                                  screen_width, screen_height, graphics::WindowMode::Windowed);

    window.set_vsync_enabled(false);
    window.set_framerate_limit(240);

    graphics::initialize_glew();    

    window.activate();
    window.clear();
    window.display();


    imgui::Context gui_context(&window);
    imgui::push_default_style();

    resources::ResourceStore resource_store;

    resource_store.car_store().load_car_directory("cars");

    game::StateMachine state_machine;
    game::LoadingThread loading_thread;

    game::GameContext game_context{};
    game_context.render_window = &window;
    game_context.gui_context = &gui_context;
    game_context.state_machine = &state_machine;
    game_context.loading_thread = &loading_thread;
    game_context.resource_store = &resource_store;

    auto& player_settings = resource_store.settings().player_settings();
    auto& cup_settings = resource_store.settings().cup_settings();

    cup::PlayerDefinition player;
    player.control_slot = 0;
    player.id = 0;
    player.name = "test";
    player_settings.selected_players.push_back(player);

    auto car_it = resource_store.car_store().car_definitions().find("f1");
    cup_settings.selected_cars.push_back(*car_it);

    if (argc >= 2)
    {
      state_machine.create_state<editor::EditorState>(game_context, argv[1]);
    }

    else
    {
      state_machine.create_state<editor::EditorState>(game_context);
    }

    game::main_loop(game_context);
  }

  catch (const std::exception& e)
  {
    std::cerr << "An unhandled exception occurred: " << e.what() << std::endl;
  }
}