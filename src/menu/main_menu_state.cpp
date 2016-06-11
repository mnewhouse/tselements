/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "main_menu_state.hpp"
#include "update_input_state.hpp"

#include "user_interface/gui_context.hpp"

namespace ts
{
  namespace menu
  {
    MainMenuState::MainMenuState(const game_context& ctx)
      : GameState(ctx),
        main_menu_(ctx.resource_store)
    {
    }

    void MainMenuState::update(const update_context& update_ctx)
    {
      main_menu_.update(input_state_, update_ctx.frame_duration);

      update_old_input_state(input_state_);
    }

    void MainMenuState::render(const render_context& render_ctx) const
    {
      gui::RenderState render_state;
      render_state.screen_size = render_ctx.screen_size;
      main_menu_.render(context().gui_context->renderer(), render_state);
    }

    void MainMenuState::process_event(const event_type& event)
    {
      update_input_state(input_state_, event);
    }
  }
}