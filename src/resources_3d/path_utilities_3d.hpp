/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "track_path_3d.hpp"

#include "utility/vector2.hpp"

#include <cstdint>

namespace ts
{
  namespace resources3d
  {
    // Interpolate the position on a path between two path nodes
    // at a specified time point.
    Vector2f path_point_at(const PathNode& a, const PathNode& b, float time_point);

    // Compute the normal of a path segment at the specified time point.
    Vector2f path_normal_at(const PathNode& a, const PathNode& b, float time_point);

    float path_width_at(const PathNode& a, const PathNode& b, float time_point);

    // Compute the path position at the given time point, offset by the normal multiplied by the
    // half path width and the relative offset parameter specified.
    inline Vector2f path_point_at(const PathNode& a, const PathNode& b,
                                  float time_point, float relative_offset);

    template <typename NodeIt>
    struct PathPoint
    {
      NodeIt node_it;
      float time_point;
      Vector2f point;
    };

    template <typename NodeIt>
    PathPoint<NodeIt>
      find_best_matching_path_position(NodeIt node_it, NodeIt node_end,
                                       Vector2f search_position, float offset, float max_distance,
                                       std::uint32_t step_count = 6, float precision = 1.0f)
    {
      PathPoint<NodeIt> result;
      result.node_it = node_end;
      result.time_point = 0.0f;

      auto max_distance_sq = max_distance * max_distance;
      auto best_distance = max_distance_sq;

      if (node_it != node_end)
      {
        const auto step = 1.0f / step_count;

        // Loop through all path segments
        auto next_it = std::next(node_it);
        for (; next_it != node_end; ++node_it, ++next_it)
        {
          const PathNode& node = *node_it;
          const PathNode& next_node = *next_it;

          // The minimum interval is defined as the specified precision divided
          // by the maximum theoretical distance between the two nodes.
          auto min_interval = precision / (distance(node.position, node.second_control) +
                                           distance(node.second_control, next_node.first_control) +
                                           distance(next_node.first_control, next_node.position));

          auto calculate_point = [&](float t)
          {
            auto width = next_node.width * t + node.width * (1.0f - t);
            auto p = path_point_at(node, next_node, t);
            p += path_normal_at(node, next_node, t) * width * 0.5f * offset;
            return p;
          };

          auto distance_squared = [&](float t)
          {
            auto p = calculate_point(t);
            auto d = search_position - p;
            return d.x * d.x + d.y * d.y;
          };

          // Coarsely find the 't' value that we're going to work with in the coming
          // binary search.
          auto t = step, best_t = 0.0f, best_d = distance_squared(best_t);
          for (std::uint32_t i = 0; i != step_count; ++i, t += step)
          {
            auto d = distance_squared(t);
            if (d < best_d)
            {
              best_t = t;
              best_d = d;
            }
          }

          // Now, iteratively reduce the interval and find the best matching 't' value.
          for (auto interval = step; interval > min_interval; )
          {
            interval *= 0.5f;

            auto min_t = std::max(best_t - interval, 0.0f);
            auto max_t = std::min(best_t + interval, 1.0f);

            auto a = distance_squared(min_t);
            auto b = distance_squared(max_t);

            if (a < best_d || b < best_d)
            {
              if (a < b)
              {
                best_d = a;
                best_t = min_t;
              }

              else
              {
                best_d = b;
                best_t = max_t;
              }
            }
          }

          // If we have found a match, and it's a better match than the one we previously found,
          // update the result variable.
          if (best_d < best_distance)
          {
            result.node_it = node_it;
            result.time_point = best_t;
            result.point = calculate_point(best_t);
            best_distance = best_d;
          }
        }
      }

      return result;
    }
  }
}