/*
 * TS Elements
 * Copyright 2015-2018 M. Newhouse
 * Released under the MIT license.
 */

#include "graphics/render_window.hpp"
#include "graphics/gl_context.hpp"

#include "fonts/builtin_fonts.hpp"
#include "fonts/font_library.hpp"

#include "game/game_context.hpp"
#include "game/loading_thread.hpp"

#include "resources/resource_store.hpp"
#include "resources/car_store.hpp"
#include "resources/settings.hpp"

#include "cup/cup_settings.hpp"

#include "client/player_settings.hpp"

#include "controls/control_center.hpp"

#include "utility/debug_log.hpp"
#include "utility/random.hpp"
#include "utility/stream_utilities.hpp"

#include <iostream>
#include <algorithm>
#include <iterator>
#include <locale>
#include <codecvt>

#include <cstdint>
#include <iostream>
#include <chrono>

using namespace ts;

int main(int argc, char** argv)
{
  /*

  debug::DebugConfig debug_config;
  debug_config.debug_level = debug::level::auxiliary;
  debug::ScopedLogger debug_log(debug_config, "debug.txt");

  try
  {
    std::locale::global(std::locale({}, new std::codecvt_utf8<wchar_t>));

    DEBUG_ESSENTIAL << "Launching program..." << debug::endl;

    int screen_width = 1280, screen_height = 800;
    graphics::RenderWindow window("TS Elements", screen_width, screen_height, graphics::WindowMode::Windowed);
    graphics::initialize_glew();

    window.clear();
    window.display();

    game::LoadingThread loading_thread;

    resources::ResourceStore resource_store;
    resource_store.car_store().load_car_directory("cars");

    for (auto font : fonts::builtin_fonts)
    {
      resource_store.font_library().load_font(font.name, font.path);
    }

    resources::TrackReference track_ref;
    track_ref.path = "tracks/TSE_Sandyshore.trk";
    track_ref.name = "Sc_Clarityre";

    auto& player_settings = resource_store.settings().player_settings();

    cup::PlayerDefinition player;
    player.control_slot = 0;
    player.id = 0;
    player.name = "test";
    player_settings.selected_players.push_back(std::move(player));

    auto& cup_settings = resource_store.settings().cup_settings();
    cup_settings.tracks.push_back(track_ref);

    auto car_it = resource_store.car_store().car_definitions().find("porge");
    cup_settings.selected_cars.push_back(*car_it);

    using StateMachine = game::StateMachine;
    StateMachine state_machine;

    game::GameContext context;
    context.state_machine = &state_machine;
    context.render_window = &window;
    context.resource_store = &resource_store;
    context.loading_thread = &loading_thread;

    state_machine.create_state<client::LocalCupState>(context);

    game::UpdateContext update_context;
    update_context.frame_duration = 20;
    const double frame_progress_multiplier = 1.0 / update_context.frame_duration;

    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::microseconds;

    std::uint64_t update_count = 0;
    std::uint64_t update_lag = 0;

    auto start_time = high_resolution_clock::now(), last_update = start_time, last_display = last_update;
    auto frame_duration = milliseconds(update_context.frame_duration);

    window.activate();
    
    while (state_machine)
    {
      // Make sure the state machine doesn't haphazardly empty itself prior to finishing this loop body.
      auto state_transition_guard = state_machine.transition_guard();

      auto time_point = high_resolution_clock::now();

      auto frame_progress = time_point - last_update;
      auto next_update = start_time + frame_duration * (update_count + update_lag);

      if (time_point >= next_update)
      {
        ++update_count;
        last_update = next_update;
        
        auto duration = time_point - last_update;
        while (duration >= frame_duration)
        {
          duration -= frame_duration;
          ++update_lag;
        }

        for (sf::Event event; window.poll_event(event);)
        {
          if (event.type == sf::Event::Closed)
          {
            // Clearing the state machine is equivalent to quitting the program.
            state_machine.clear();
          }

          state_machine->process_event(event);
        }

        frame_progress = duration;
        state_machine->update(update_context);
      }

      game::RenderContext render_context;
      render_context.screen_size = window.size();
      render_context.frame_progress = duration_cast<milliseconds>(frame_progress).count() * frame_progress_multiplier;
      if (render_context.frame_progress > 1.0) render_context.frame_progress = 1.0;

      window.clear({ 0.0f, 0.0f, 0.0f, 1.0f });
      state_machine->render(render_context);

      window.display();
    }
  }

  catch (const std::exception& error)
  {
    std::cerr << error.what() << std::endl;
    DEBUG_ESSENTIAL << "A fatal error has caused the program to terminate: " << error.what() << debug::endl;

    // TODO: Do something better than crash
    throw;
  }
  */

  return 0;
}

#include "core/config_definitions.hpp"