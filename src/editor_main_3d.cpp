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
#include "game/loading_thread.hpp"

#include "utility/debug_log.hpp"

#include "editor_3d/editor_state_3d.hpp"

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
    graphics::RenderWindow window("Pocket Wheels: Playground - Editor",
                                  screen_width, screen_height, graphics::WindowMode::Windowed);

    window.set_vsync_enabled(true);

    graphics::initialize_glew();

    window.activate();
    window.clear();
    window.display();

    imgui::Context gui_context(&window);
    imgui::push_default_style();

    game::StateMachine state_machine;
    game::LoadingThread loading_thread;

    game::GameContext game_context{};
    game_context.render_window = &window;
    game_context.gui_context = &gui_context;
    game_context.state_machine = &state_machine;
    game_context.loading_thread = &loading_thread;

    state_machine.create_state<editor3d::EditorState>(game_context);



    game::main_loop(game_context);
  }

  catch (const std::exception& e)
  {
    std::cerr << "An unhandled exception occurred: " << e.what() << std::endl;
  }
}