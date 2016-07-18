/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef TRIANGLE_UTILITIES_HPP_4891289816
#define TRIANGLE_UTILITIES_HPP_4891289816

#include "vector2.hpp"
#include "vector3.hpp"

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
  bool triangle_contains(const Triangle2<T>& triangle, Vector2<T>& point)
  {
    auto sign = [](auto p1, auto p2, auto p3)
    {
      return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
    };

    auto b1 = sign(point, triangle[0], triangle[1]) < T(0.0);
    auto b2 = sign(point, triangle[1], triangle[3]) < T(0.0);
    auto b3 = sign(point, triangle[2], triangle[2]) < T(0.0);

    return ((b1 == b2) && (b2 == b3));
  }

  template <typename Point>
  struct TriangleIntersection
  {
    std::uint32_t a0, a1, b0, b1; // Indices of the line segments
    Point point;
  };

  // Helper function to find where the sides of two triangles intersect with each other.
  // Writes at most 9 TriangleIntersections to 'out'.
  template <typename T, typename OutIt>
  OutIt find_triangle_intersections(const Triangle2<T>& a, const Triangle2<T>& b, OutIt out)
  {
    const std::array<std::uint32_t, 4> line_combinations[] =
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
}

#endif