/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "path_utilities_3d.hpp"

namespace ts
{
  namespace resources3d
  {
    // Interpolate the position on a path between two path nodes
    // at a specified time point.
    Vector2f path_point_at(const PathNode& a, const PathNode& b, float time_point)
    {
      auto t = time_point, r = 1.0f - t;
      auto tt = t * t, ttt = tt * t;
      auto rr = r * r, rrr = rr * r;

      return rrr * a.position + 3.0f * rr * t * a.second_control +
        3.0f * r * tt * b.first_control + ttt * b.position;
    }

    // Compute the normal of a path segment at the specified time point.
    Vector2f path_normal_at(const PathNode& a, const PathNode& b, float time_point)
    {
      auto t = time_point, r = 1.0f - time_point;
      auto tt = t * t, rr = r * r;

      auto d = 3.0f * rr * (a.second_control - a.position) + 6.0f * r * t * (b.first_control - a.second_control)
        + 3.0f * tt * (b.position - b.first_control);

      return normalize(make_vector2(-d.y, d.x));
    }

    float path_width_at(const PathNode& a, const PathNode& b, float time_point)
    {
      return a.width * (1.0f - time_point) + b.width * time_point;
    }

    // Compute the path position at the given time point, offset by the normal multiplied by the
    // half path width and the relative offset parameter specified.
    Vector2f path_point_at(const PathNode& a, const PathNode& b,
                                  float time_point, float relative_offset)
    {
      auto point = path_point_at(a, b, time_point);
      auto normal = path_normal_at(a, b, time_point);
      auto width = path_width_at(a, b, time_point);

      return point + normal * width * 0.5f * relative_offset;
    }
  }
}