/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "utility/vector2.hpp"
#include "utility/color.hpp"

#include <vector>

namespace ts
{
  namespace resources
  {
    struct TrackPathNode
    {
      Vector2f first_control;
      Vector2f position;      
      Vector2f second_control;

      float width = 0.0f;
    };

    struct SubPath
    {
      using Node = TrackPathNode;
      bool closed = false;
      std::vector<Node> nodes;
    };

    struct TrackPath
    {
      std::uint32_t id = 0;
      std::vector<SubPath> sub_paths;
    };

    struct StrokeSegment
    {
      std::uint32_t sub_path_id;
      float start_time_point;
      float end_time_point;

      enum Side
      {
        First,
        Second
      } side;
    };

    struct PathStyle
    {
      std::uint32_t preset_id = 0;
      std::uint32_t base_texture;
      std::uint32_t border_texture;
      std::uint32_t terrain_id;
      bool is_segmented = false;
      bool border_only = false;
      float fade_length = 0.0f;
      float width = 64.0f;
      float border_width = 1.0f;
      Vector2f base_texture_tile_size = { 256.0f, 256.0f };
      Vector2f border_texture_tile_size = { 256.0f, 256.0f };

      enum TextureMode
      {
        Tiled,
        Directional
      } texture_mode = Tiled;
      std::vector<StrokeSegment> segments;
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


    inline Vector2f path_direction_at(const TrackPathNode& a, const TrackPathNode& b, float time_point)
    {
      auto t = time_point, r = 1.0f - time_point;
      auto tt = t * t, rr = r * r;

      auto d = 3.0f * rr * (a.second_control - a.position) + 6.0f * r * t * (b.first_control - a.second_control)
        + 3.0f * tt * (b.position - b.first_control);

      return normalize(d);
    }

    inline Vector2f path_normal_at(const TrackPathNode& a, const TrackPathNode& b, float time_point)
    {
      auto d = path_direction_at(a, b, time_point);
      return normalize(make_vector2(-d.y, d.x));
    }

    inline float path_width_at(const TrackPathNode& a, const TrackPathNode& b, float time_point)
    {
      return a.width * (1.0f - time_point) + b.width * time_point;
    }

    inline Vector2f path_point_at(const SubPath& path, float time_point, float offset)
    {
      if (path.nodes.empty()) return{};

      auto idx = static_cast<std::size_t>(time_point);
      auto next_idx = idx + 1;

      if (path.closed)
      {
        while (idx >= path.nodes.size()) idx -= path.nodes.size();
        while (next_idx >= path.nodes.size()) next_idx -= path.nodes.size();
      }

      else
      {
        idx = std::min(idx, path.nodes.size() - 1);
        next_idx = std::min(next_idx, path.nodes.size() - 1);
      }

      const auto& a = path.nodes[idx];
      const auto& b = path.nodes[next_idx];

      auto t = time_point - std::floor(time_point);

      auto w = path_width_at(a, b, t) * 0.5f;
      if (offset < 0.0) offset -= w;
      else offset += w;

      return path_point_at(a, b, t) + path_normal_at(a, b, t) * offset;
    }
  }
}
