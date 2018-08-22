/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "vector2.hpp"
#include "vector3.hpp"
#include "rect.hpp"
#include "line_intersection.hpp"

#include <array>

namespace ts
{
  template <typename T>
  using Triangle2 = std::array<Vector2<T>, 3>;

  using Triangle2f = Triangle2<float>;

  template <typename T>
  using Triangle3 = std::array<Vector3<T>, 3>;

  using Triangle3f = Triangle2<float>;

  template <typename T>
  Triangle3<T> make_triangle(const Vector3<T>& a, const Vector3f& b, const Vector3f& c)
  {
    return{ a, b, c };
  }

  template <typename T>
  Triangle2<T> make_triangle(const Vector2<T>& a, const Vector2<T>& b, const Vector2<T>& c)
  {
    return{ a, b, c };
  }
  
  template <typename T>
  bool triangle_contains(const Triangle2<T>& triangle, const Vector2<T>& point)
  {
    auto t0 = triangle[0], t1 = triangle[1], t2 = triangle[2];

    auto s = t0.y * t2.x - t0.x * t2.y + (t2.y - t0.y) * point.x + (t0.x - t2.x) * point.y;
    auto t = t0.x * t1.y - t0.y * t1.x + (t0.y - t1.y) * point.x + (t1.x - t0.x) * point.y;

    if ((s < 0) != (t < 0))
      return false;

    auto a = -t1.y * t2.x + t0.y * (t2.x - t1.x) + t0.x * (t1.y - t2.y) + t1.x * t2.y;
    if (a < 0)
    {
      s = -s;
      t = -t;
      a = -a;
    }

    return s > 0 && t > 0 && (s + t) <= a;
  }

  template <typename T>
  bool triangle_contains_inclusive(const Triangle2<T>& triangle, const Vector2<T>& point)
  {
    auto t0 = triangle[0], t1 = triangle[1], t2 = triangle[2];

    auto s = t0.y * t2.x - t0.x * t2.y + (t2.y - t0.y) * point.x + (t0.x - t2.x) * point.y;
    auto t = t0.x * t1.y - t0.y * t1.x + (t0.y - t1.y) * point.x + (t1.x - t0.x) * point.y;

    if ((s < 0) != (t < 0))
      return false;

    auto a = -t1.y * t2.x + t0.y * (t2.x - t1.x) + t0.x * (t1.y - t2.y) + t1.x * t2.y;
    if (a < 0)
    {
      s = -s;
      t = -t;
      a = -a;
    }

    return s >= 0 && t >= 0 && (s + t) <= a;
  }

  template <typename Point>
  struct TriangleIntersection
  {
    std::uint8_t a0, a1, b0, b1; // Indices of the line segments
    Point point;
  };

  template <typename T>
  auto triangle_area(const Triangle2<T>& triangle)
  {
    using std::abs;
    return abs(cross_product(triangle[1] - triangle[0], triangle[2] - triangle[0])) / 2;
  }

  // Helper function to find where the sides of two triangles intersect with each other.
  // Writes at most 9 TriangleIntersections to 'out'.
  template <typename T, typename OutIt>
  OutIt find_triangle_intersections(const Triangle2<T>& a, const Triangle2<T>& b, OutIt out)
  {
    const std::array<std::uint8_t, 4> line_combinations[] =
    {
      { 0, 1, 0, 1 },
      { 0, 1, 1, 2 },
      { 0, 1, 2, 0 },
      { 1, 2, 0, 1 },
      { 1, 2, 1, 2 },
      { 1, 2, 2, 0 },
      { 2, 0, 0, 1 },
      { 2, 0, 1, 2 },
      { 2, 0, 2, 0 }
    };

    using point_type = std::decay_t<decltype(a[0])>;

    for (const auto combination : line_combinations)
    {
      const auto a0 = combination[0], a1 = combination[1];
      const auto b0 = combination[2], b1 = combination[3];

      if (auto intersection_point = find_line_intersection(a[a0], a[a1], b[b0], b[b1]))
      {
        TriangleIntersection<point_type> intersection = { a0, a1, b0, b1, *intersection_point };
        *out++ = intersection;
      }
    }

    return out;
  }

  // Winding order must be such that cross_product(t2 - t1, t3 - t1) < 0
  template <typename T>
  bool region_contains_triangle(Rect<T> region, Vector2<T> t1, Vector2<T> t2, Vector2<T> t3)
  {
    auto cross = [](auto a, auto b, auto p)
    {
      return cross_product(b - a, p - a);
    };

    if (cross(t1, t2, t3) > 0)
    {
      std::swap(t1, t3);
    }

    auto top_left = make_vector2(region.left, region.top);
    auto bottom_right = make_vector2(region.right(), region.bottom());
    auto top_right = make_vector2(bottom_right.x, top_left.y);
    auto bottom_left = make_vector2(top_left.x, bottom_right.y);

    auto test_edge = [=](auto a, auto b)
    {
      return cross(a, b, top_left) < 0 ||
        cross(a, b, bottom_left) < 0 ||
        cross(a, b, bottom_right) < 0 ||
        cross(a, b, top_right) < 0;
    };

    return test_edge(t1, t2) && test_edge(t2, t3) && test_edge(t3, t1);
  }
}
