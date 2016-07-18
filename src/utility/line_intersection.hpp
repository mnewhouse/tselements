/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef LINE_INTERSECTION_578111845_HPP
#define LINE_INTERSECTION_578111845_HPP

#include "vector2.hpp"

#include <boost/optional.hpp>

#include <array>
#include <type_traits>

namespace ts
{
  // Finds the intersection point of two lines. Returns boost::none of the lines are parallel or
  // if they lines don't intersect with each other, and the point of intersection otherwise.
  template <typename T>
  boost::optional<Vector2<T>> find_line_intersection(Vector2<T> a1, Vector2<T> a2, Vector2<T> b1, Vector2<T> b2,
                                                     bool a_segment_test = true, bool b_segment_test = true)
  {
    auto d = (a1.x - a2.x) * (b1.y - b2.y) - (a1.y - a2.y) * (b1.x - b2.x);
    if (d == 0) return boost::none;

    auto a = (a1.x * a2.y - a1.y * a2.x);
    auto b = (b1.x * b2.y - b1.y * b2.x);

    Vector2<T> result =
    {
      ((b1.x - b2.x) * a - (a1.x - a2.x) * b) / d,
      ((b1.y - b2.y) * a - (a1.y - a2.y) * b) / d
    };

    // See if the intersection point is within both lines' bounding boxes
    if (a_segment_test && !contains_inclusive(make_rect_from_points(a1, a2), result) ||
        b_segment_test && !contains_inclusive(make_rect_from_points(b1, b2), result))
    {
      return boost::none;
    }

    return result;
  }
}

#endif