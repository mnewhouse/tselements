/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef GUI_MOUSE_HPP_412583912
#define GUI_MOUSE_HPP_412583912

#include <cstdint>

namespace ts
{
  namespace gui
  {
    enum class MouseButton
      : std::uint32_t
    {
      None = 0,
      Left = 1,
      Right = 2,
      Middle = 4
    };
  }
}

#endif