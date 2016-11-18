/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "vector2.hpp"
#include "color.hpp"

namespace ts
{
  template <typename PositionType, typename ColorType>
  struct Vertex
  {
    Vector2<PositionType> position;
    Vector2<PositionType> texture_coords;
    Color<ColorType> color;
  };

  template <typename PositionType>
  struct UncoloredVertex
  {
    Vector2<PositionType> position;
    Vector2<PositionType> texture_coords;
  };
}
