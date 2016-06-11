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

    struct TrackPathStroke
    {
      std::uint16_t texture_id;
      Colorb color = { 255, 255, 255, 255 };

      bool use_relative_size = true;
      float width = 1.0f;
      float offset = 0.0f;

      enum Type
      {
        Default, Border
      } type = Default;
    };

    struct TrackPath
    {
      using Node = TrackPathNode;
      std::vector<Node> nodes;

      std::vector<TrackPathStroke> strokes;
    };

    inline Vector2f path_point_at(const TrackPathNode& a, const TrackPathNode& b, float time_point)
    {
      auto t = time_point, r = 1.0f - t;
      auto tt = t * t, ttt = tt * t;
      auto rr = r * r, rrr = rr * r;

      return rrr * a.position + 3.0f * rr * t * a.second_control +
        3.0f * r * tt * b.first_control + ttt * b.position;
    }

    inline Vector2f path_normal_at(const TrackPathNode& a, const TrackPathNode& b, float time_point)
    {
      auto t = time_point, r = 1.0f - time_point;
      auto tt = t * t, rr = r * r;

      auto d = 3.0f * rr * (a.second_control - a.position) + 6.0f * r * t * (b.first_control - a.second_control)
        + 3.0f * tt * (b.position - b.first_control);

      return normalize(make_vector2(-d.y, d.x));
    }
  }
}

#endif