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
    namespace mouse_button
    {
      enum MouseButton
        : std::uint32_t
      {
        none = 0,
        left = 1,
        right = 2,
        middle = 4
      };
    }


    struct InputState
    {
      bool updated = false;

      Vector2i mouse_position;
      Vector2i old_mouse_position;
      Vector2i click_position;

      std::int32_t mouse_wheel_delta = 0;
      std::uint32_t old_mouse_button_state = 0;
      std::uint32_t mouse_button_state = 0;     
    };
  }
}

#endif