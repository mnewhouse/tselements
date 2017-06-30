/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "vector2.hpp"

#include <array>
#include <type_traits>
#include <algorithm>

namespace ts
{
  template <typename T>
  struct LineIntersection
  {
    bool parallel = false;
    bool a_test = false;
    bool b_test = false;
    bool intersected = false;
    Vector2<T> point;
    
    explicit operator bool() const
    {
      return intersected;
    }
  };

  // Finds the intersection point of two lines. Returns boost::none of the lines are parallel or
  // if they lines don't intersect with each other, and the point of intersection otherwise.
  template <typename T>
  LineIntersection<T> find_line_intersection(Vector2<T> a1, Vector2<T> a2, Vector2<T> b1, Vector2<T> b2)
  {
    LineIntersection<T> result;

    auto d = (a1.x - a2.x) * (b1.y - b2.y) - (a1.y - a2.y) * (b1.x - b2.x);
    if (d == 0)
    {
      result.parallel = true;
      return result;
    }

    auto a = (a1.x * a2.y - a1.y * a2.x);
    auto b = (b1.x * b2.y - b1.y * b2.x);

    result.point =
    {
      ((b1.x - b2.x) * a - (a1.x - a2.x) * b) / d,
      ((b1.y - b2.y) * a - (a1.y - a2.y) * b) / d
    };

    return result;
  }

  template <typename T>
  LineIntersection<T> find_line_segment_intersection(Vector2<T> a1, Vector2<T> a2, Vector2<T> b1, Vector2<T> b2)
  {
    auto minmax_ax = std::minmax(a1.x, a2.x);
    auto minmax_ay = std::minmax(a1.y, a2.y);

    auto minmax_bx = std::minmax(b1.x, b2.x);
    auto minmax_by = std::minmax(b1.y, b2.y);

    LineIntersection<T> result;
    if (minmax_ax.second <= minmax_bx.first || minmax_ay.second <= minmax_by.first ||
        minmax_bx.second <= minmax_ax.first || minmax_by.second <= minmax_ay.first)
    {
      return result;
    }

    result = find_line_intersection(a1, a2, b1, b2);
    if (result.parallel) return result;    

    const auto& point = result.point;

    // See if the intersection point is within the lines' bounding boxes
    result.a_test = point.x > minmax_ax.first && point.y > minmax_ay.first &&
      point.x < minmax_ax.second && point.y < minmax_ay.second;

    result.b_test = point.x > minmax_bx.first && point.y > minmax_by.first &&
      point.x < minmax_bx.second && point.y < minmax_by.second;

    result.intersected = result.a_test && result.b_test;

    return result;
  }
}
