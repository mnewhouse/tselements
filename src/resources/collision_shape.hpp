/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "utility/rect.hpp"
#include "utility/vector2.hpp"

#include <boost/container/small_vector.hpp>
#include <boost/variant.hpp>

#include <cstdint>
#include <array>

namespace ts
{
  namespace resources
  {
    namespace collision_shapes
    {
      struct Circle
      {
        Vector2f center;
        float radius;
        std::uint32_t height = 1;
      };

      struct Polygon
      {
        struct Point
        {
          Vector2f position;
        };

        std::array<Point, 8> points;
        std::uint32_t num_points;
        std::uint32_t height = 1;
      };

      struct SubShape
      {
        boost::variant<Circle, Polygon> data;
        float bounciness;
      };
    }

    struct CollisionShape
    {
      boost::container::small_vector<collision_shapes::SubShape, 4> sub_shapes;
    };
  }
}