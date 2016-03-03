/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef START_POINT_HPP_58912835
#define START_POINT_HPP_58912835

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

#endif