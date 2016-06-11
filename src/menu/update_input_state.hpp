/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef UPDATE_INPUT_STATE_HPP_38419283489
#define UPDATE_INPUT_STATE_HPP_38419283489

#include "game/game_events.hpp"

#include "user_interface/gui_input_state.hpp"

namespace ts
{
  namespace menu
  {
    void update_input_state(gui::InputState& input_state, const game::Event& event);
    void update_old_input_state(gui::InputState& input_state);
  }
}

#endif