/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/


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

      auto start_time = high_resolution_clock::now(), last_frame = start_time;

      auto frame_duration = milliseconds(update_context.frame_duration);

      high_resolution_clock::duration accumulator = frame_duration;

      std::uint64_t frame_id = 0;

      if (window) window->activate();     

      while (state_machine)
      {
        auto time_point = high_resolution_clock::now();

        auto frame_time = (time_point - last_frame);
        last_frame = time_point;

        accumulator += frame_time;
        auto frame_accumulator = accumulator + microseconds(16667);

        if (frame_accumulator >= frame_duration)
        {
          auto state_transition_guard = state_machine.transition_guard();

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

          if (frame_accumulator >= frame_duration)
          {
            while (frame_accumulator >= frame_duration)
            {
              accumulator -= frame_duration;
              frame_accumulator -= frame_duration;
            }

            if (gui_context) gui_context->new_frame(update_context.frame_duration);
            state_machine->update(update_context);            
          }
        }

        if (window)
        {
          game::RenderContext render_context;
          render_context.screen_size = window->size();

          auto ms_elapsed = duration_cast<milliseconds>(frame_accumulator).count();
          render_context.frame_progress = ms_elapsed * frame_progress_multiplier;          

          window->clear();

          if (state_machine) state_machine->render(render_context);
          if (gui_context) gui_context->render();  

          window->display();
        }
      }
    }    
  }
}