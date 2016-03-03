/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CONTROL_POINT_HPP_112039581298
#define CONTROL_POINT_HPP_112039581298

#include "utility/vector2.hpp"

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

      Vector2i start;
      Vector2i end;
      Type type;
    };
  }
}

#endif