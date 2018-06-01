/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "utility/vector2.hpp"
#include "utility/vector3.hpp"
#include "utility/math_utilities.hpp"

#include <vector>
#include <cstdint>

namespace ts
{
  namespace resources3d
  {
    class ElevationMap
    {
    public:
      explicit ElevationMap(Vector2i grid_size, std::int32_t cell_size, float elevation = 0.0f, float max_elevation = 1000.f);

      void set_elevation_at(std::int32_t x, std::int32_t y, float elevation)
      {
        elevation = utility::clamp(elevation, 0.0f, elevation_limit_);        

        if (elevation > highest_elevation_) highest_elevation_ = elevation;
        if (elevation < lowest_elevation_) lowest_elevation_ = elevation;

        map_data_[y * grid_size_.x + x] = elevation;
      }

      float elevation_at(std::int32_t x, std::int32_t y) const
      {
        x += cell_offset_.x;
        y += cell_offset_.y;

        return map_data_[y * grid_size_.x + x];
      }

      float elevation_at(Vector2i cell) const
      {
        return elevation_at(cell.x, cell.y);
      }

      Vector2i grid_size() const
      {
        return grid_size_;
      }

      Vector2i cell_offset() const
      {
        return cell_offset_;
      }

      void set_cell_offset(Vector2i offset)
      {
        cell_offset_ = offset;
      }

      std::int32_t cell_size() const
      {
        return cell_size_;
      }

      float elevation_limit() const
      {
        return elevation_limit_;
      }

      float highest_elevation() const
      {
        return highest_elevation_;
      }

      float lowest_elevation() const
      {
        return lowest_elevation_;
      }


    private:
      std::vector<float> map_data_;
      Vector2i grid_size_;
      Vector2i cell_offset_;
      std::int32_t cell_size_;

      float elevation_limit_;
      float highest_elevation_;
      float lowest_elevation_;
    };

    ElevationMap generate_random_elevation_map(Vector2i minimum_size, std::int32_t cell_size,
                                               float base_elevation, float max_elevation, float variance);

    float interpolate_elevation_at(const ElevationMap& elevation_map, Vector2f position);

    Vector3f interpolate_terrain_normal_at(const ElevationMap& elevation_map, Vector2f position);
  } 
}