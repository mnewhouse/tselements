/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef GUI_INPUT_STATE_HPP_78921178
#define GUI_INPUT_STATE_HPP_78921178

#include "utility/vector2.hpp"

namespace ts
{
  namespace gui
  {
    enum class MouseButton
      : std::uint32_t
    {
      Left = 1,
      Right = 2,
      Middle = 4
    };

    struct InputState
    {
      Vector2i mouse_position;
      Vector2i mouse_delta;
      std::int32_t mouse_wheel_delta = 0;
      std::uint32_t old_mouse_button_state = 0;
      std::uint32_t mouse_button_state = 0;
    }
  }
}

#endif