/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef GUI_INPUT_STATE_HPP_78921178
#define GUI_INPUT_STATE_HPP_78921178

#include "gui_mouse.hpp"

#include "utility/vector2.hpp"

namespace ts
{
  namespace gui
  {
    struct InputState
    {
      //bool updated = false;

      Vector2i mouse_position = { -1, -1 };
      Vector2i old_mouse_position = { -1, -1 };
      Vector2i click_position = { -1, -1 };

      std::int32_t mouse_wheel_delta = 0;
      std::uint32_t old_mouse_button_state = 0;
      std::uint32_t mouse_button_state = 0;     
    };

    inline bool click_state(const InputState& input_state, MouseButton button)
    {
      return (input_state.mouse_button_state & static_cast<std::uint32_t>(button)) != 0;
    }

    inline bool old_click_state(const InputState& input_state, MouseButton button)
    {
      return (input_state.old_mouse_button_state & static_cast<std::uint32_t>(button)) != 0;
    }

    inline bool was_clicked(const InputState& input_state, 
                            gui::MouseButton button = gui::MouseButton::Left)
    {
      return old_click_state(input_state, button) && !click_state(input_state, button);
    }
  }
}

#endif