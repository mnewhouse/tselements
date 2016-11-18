/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "vector2.hpp"
#include "rect.hpp"
#include "rotation.hpp"

#include <cmath>

namespace ts
{
  template <typename T>
  Vector2<T> transform_point(const Vector2<T>& point, T sin, T cos)
  {
    return{ point.x * cos - sin * point.y, point.y * cos + sin * point.x };
  }

  template <typename T>
  Vector2<T> transform_point(const Vector2<T>& point, Rotation<T> rotation)
  {
    using std::sin;
    using std::cos;

    auto rad = rotation.radians();
    return transform_point(point, sin(rad), cos(rad));
  }

  template <typename T>
  Rect<T> transform_rect(const Rect<T>& rect, T sin, T cos)
  {
    Vector2<T> center(rect.left + rect.width * 0.5, rect.top + rect.height * 0.5);
    T left = rect.left - center.x;
    T top = rect.top - center.y;
    T right = left + rect.width;
    T bottom = top + rect.height;

    Vector2<T> points[4] =
    {
      transform_point<T>({ left, top }, sin, cos) + center,
      transform_point<T>({ left, bottom }, sin, cos) + center,
      transform_point<T>({ right, bottom }, sin, cos) + center,
      transform_point<T>({ right, top }, sin, cos) + center
    };

    left = points[0].x;
    right = points[0].x;
    top = points[0].y;
    bottom = points[0].y;
    for (int i = 1; i != 4; ++i)
    {
      if (points[i].x < left) left = points[i].x;
      else if (points[i].x > right) right = points[i].x;

      if (points[i].y < top) top = points[i].y;
      else if (points[i].y > bottom) bottom = points[i].y;
    }

    return Rect<T>(left, top, right - left, bottom - top);
  }

  template <typename T>
  Rect<T> transform_rect(const Rect<T>& rect, Rotation<T> rotation)
  {
    using std::sin;
    using std::cos;

    auto rad = rotation.radians();
    return transform_rect(rect, sin(rad), cos(rad));
  }
}
