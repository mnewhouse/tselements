/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef TRACK_PATH_HPP_182981
#define TRACK_PATH_HPP_182981

#include "utility/vector2.hpp"
#include "utility/color.hpp"

#include <vector>

#include <boost/iterator/transform_iterator.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/range/iterator_range.hpp>

namespace ts
{
  namespace resources_3d
  {
    struct TrackPathNode
    {
      Vector2f first_control;
      Vector2f position;      
      Vector2f second_control;

      float width = 0.0;
    };

    struct StrokeProperties
    {
      std::uint16_t texture_id;
      Colorb color = { 255, 255, 255, 255 };

      bool is_segmented = false;
      bool use_relative_size = true;
      float width = 1.0f;
      float offset = 0.0f;
      float texture_scale = 0.5f;

      float inner_normal = 0.0f;
      float outer_normal = 0.0f;
      
      float bevel_width = 0.0f;
      float bevel_strength = 0.5f;

      enum TextureMode
      {
        Tiled,
        Directional
      } texture_mode = Tiled;

      enum Type
      {
        Default, Border
      } type = Default;
    };

    struct StrokeSegment
    {
      std::uint32_t start_index;
      std::uint32_t end_index;

      float start_time_point;
      float end_time_point;

      enum Side
      {
        First,
        Second,
        Both
      } side;
    };

    struct SegmentedStroke
    {
      SegmentedStroke() = default;
      SegmentedStroke(StrokeProperties properties_)
        : properties(std::move(properties_))
      {}

      StrokeProperties properties;
      std::vector<StrokeSegment> segments;
    };

    struct TrackPath
    {
      using Node = TrackPathNode;
      bool closed = false;

      float min_width = 56.0f;
      float max_width = 56.0f;

      std::vector<Node> nodes;
      std::vector<SegmentedStroke> strokes;
    };

    // Interpolate the position on a path between two path nodes
    // at a specified time point.
    inline Vector2f path_point_at(const TrackPathNode& a, const TrackPathNode& b, float time_point)
    {
      auto t = time_point, r = 1.0f - t;
      auto tt = t * t, ttt = tt * t;
      auto rr = r * r, rrr = rr * r;

      return rrr * a.position + 3.0f * rr * t * a.second_control +
        3.0f * r * tt * b.first_control + ttt * b.position;
    }

    // Compute the normal of a path segment at the specified time point.
    inline Vector2f path_normal_at(const TrackPathNode& a, const TrackPathNode& b, float time_point)
    {
      auto t = time_point, r = 1.0f - time_point;
      auto tt = t * t, rr = r * r;

      auto d = 3.0f * rr * (a.second_control - a.position) + 6.0f * r * t * (b.first_control - a.second_control)
        + 3.0f * tt * (b.position - b.first_control);

      return normalize(make_vector2(-d.y, d.x));
    }

    template <typename NodeIt>
    struct PathPoint
    {
      NodeIt node_it;
      float time_point;
      Vector2f point;
    };

    template <typename NodeIt>
    PathPoint<NodeIt>
      find_first_matching_path_position(NodeIt node_it, NodeIt node_end,                            
                                        Vector2f search_position, float offset, float max_distance,
                                        std::uint32_t step_count = 6, float precision = 1.0f)
    {
      if (node_it != node_end)
      {
        auto max_distance_sq = max_distance * max_distance;
        const auto step = 1.0f / step_count;

        // Loop through all path segments
        auto next_it = std::next(node_it);
        for (; next_it != node_end; ++node_it, ++next_it)
        {
          const TrackPathNode& node = *node_it;
          const TrackPathNode& next_node = *next_it;

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

          // If we have found a match, there's no sense lingering around here
          // and we can just return the thing.
          if (best_d < max_distance_sq)
          {
            return{ node_it, best_t, calculate_point(best_t) };
          }
        }
      }

      return{ node_end, 0.0f, Vector2f() };
    }
  }
}

#endif
