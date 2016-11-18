/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "utility/vector2.hpp"

namespace ts
{
  namespace resources
  {
    struct StartPoint
    {
      Vector2i position;
      std::int32_t rotation = 0; // degrees
      std::uint32_t level = 0;
    };
  }
}
