/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"

#include "main_loop.hpp"
#include "game_state.hpp"

#include "graphics/render_window.hpp"

#include "imgui/imgui_sfml_opengl.hpp"

#include <chrono>

namespace ts
{
  namespace game
  {
    void main_loop(const GameContext& game_context)
    {
      auto& state_machine = *game_context.state_machine;
      auto window = game_context.render_window;
      auto gui_context = game_context.gui_context;

      using std::chrono::high_resolution_clock;
      using std::chrono::duration_cast;
      using std::chrono::milliseconds;
      using std::chrono::microseconds;

      game::UpdateContext update_context{};
      update_context.frame_duration = 20U;

      const double frame_progress_multiplier = 1.0 / update_context.frame_duration;

      std::uint64_t update_count = 0;
      std::uint64_t update_lag = 0;

      auto start_time = high_resolution_clock::now(), last_update = start_time, last_display = start_time;
      auto frame_duration = milliseconds(update_context.frame_duration);

      if (window) window->activate();

      while (state_machine)
      {
        auto time_point = high_resolution_clock::now();
        auto render_time = (time_point - last_display);

        if (render_time >= milliseconds(20))
        {
          std::printf("%llu\n", render_time.count());
        }

        last_display = time_point;
        time_point += render_time;        

        auto frame_progress = time_point - last_update;
        auto next_update = start_time + frame_duration * (update_count + update_lag);  

        // See if the time point plus the expected render time is later than the next scheduled update.
        if (time_point >= next_update)
        {
          auto state_transition_guard = state_machine.transition_guard();

          ++update_count;
          last_update = next_update;

          auto duration = time_point - last_update;
          while (duration >= frame_duration)
          {
            duration -= frame_duration;
            ++update_lag;
          }

          if (window)
          {
            for (sf::Event event; window->poll_event(event);)
            {
              if (event.type == sf::Event::Closed)
              {
                // Clearing the state machine is equivalent to quitting the program.
                state_machine.clear();
              }

              bool process = window->has_focus();

              // Certain events should be processed regardless of whether the window has focus
              if (event.type == sf::Event::GainedFocus || event.type == sf::Event::LostFocus) process = true;

              if (process)
              {
                if (gui_context) gui_context->process_event(event);
                state_machine->process_event(event);
              }
            }
          }

          frame_progress = duration;

          if (gui_context) gui_context->new_frame(update_context.frame_duration);

          state_machine->update(update_context);                  
        }

        if (window)
        {
          game::RenderContext render_context;
          render_context.screen_size = window->size();
          render_context.frame_progress = duration_cast<milliseconds>(frame_progress).count() * frame_progress_multiplier;
          if (render_context.frame_progress > 1.0) render_context.frame_progress = 1.0;

          window->clear();

          if (state_machine) state_machine->render(render_context);
          if (gui_context) gui_context->render();

          window->display();
        }
      }
    }
  }
}