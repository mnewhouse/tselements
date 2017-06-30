/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "utility/vector2.hpp"
#include "utility/color.hpp"

#include <vector>

namespace ts
{
  namespace resources3d
  {
    struct PathNode
    {
      Vector2f first_control;
      Vector2f position;
      Vector2f second_control;

      float width = 0.0;
    };

    struct PathStrokeStyle
    {
      enum Type
      {
        Regular, Border
      };

      Type type = Regular;
      Colorb color = Colorb(255, 255, 255, 255);
      float offset = 0.0f;      
    };

    struct Path
    {
      using Node = PathNode;
      bool closed = false;

      float min_width = 56.0f;
      float max_width = 56.0f;

      std::vector<Node> nodes;
      std::vector<PathStrokeStyle> stroke_styles;
    };

    struct PathLayer
    {
      std::vector<Path> paths;
    };
  }
}