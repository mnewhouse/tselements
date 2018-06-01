/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "utility/vector2.hpp"

#include <cstdint>

namespace ts
{
  namespace resources
  {
    struct ControlPoint
    {
      enum Type
      {
        FinishLine, HorizontalLine, VerticalLine, Area,
      };

      enum Flags : std::uint32_t
      {
        None = 0,
        Sector = 1,
        SpeedTest = 2
      };

      Vector2i start;
      Vector2i end;
      Type type;
      std::uint32_t flags;
    };
  }
}