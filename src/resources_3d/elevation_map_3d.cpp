/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "elevation_map_3d.hpp"

#include "utility/random.hpp"
#include "utility/rect.hpp"
#include "utility/math_utilities.hpp"

#include <numeric>

namespace ts
{
  namespace resources3d
  {
    ElevationMap::ElevationMap(Vector2i grid_size, std::int32_t cell_size, float base_elevation, float elevation_limit)
      : map_data_(grid_size.x * grid_size.y, base_elevation),
        grid_size_(grid_size),
        cell_size_(cell_size),
        elevation_limit_(elevation_limit),
        highest_elevation_(base_elevation),
        lowest_elevation_(base_elevation)
    {
    }

    ElevationMap generate_random_elevation_map(Vector2i minimum_size, std::int32_t cell_size,
                                               float base_elevation, float max_elevation, float variance)
    {
      const auto max_axis = std::max(minimum_size.x, minimum_size.y);
      const auto map_size = utility::next_power_of_two((max_axis + cell_size - 1) / cell_size);
      Vector2i cell_offset = (minimum_size - map_size) / 2;

      ElevationMap map({ map_size + 1, map_size + 1 }, cell_size);

      {
        // Generate random elevation at the four corners of the map
        std::uniform_real_distribution<float> dist(base_elevation, max_elevation);

        map.set_elevation_at(0, 0, utility::random_number(dist));
        map.set_elevation_at(map_size, 0, utility::random_number(dist));
        map.set_elevation_at(map_size, map_size, utility::random_number(dist));
        map.set_elevation_at(0, map_size, utility::random_number(dist));
      }

      // Basic square-triangle algorithm implementation
      variance *= (max_elevation - base_elevation) * 0.5f;
      for (auto square_size = map_size; square_size > 1; square_size /= 2, variance *= 0.5f)
      {
        const auto half_square = square_size / 2;
        std::uniform_real_distribution<float> dist(-variance, variance);

        for (std::int32_t y = 0; y < map_size; y += square_size)
        {
          for (std::int32_t x = 0; x < map_size; x += square_size)
          {
            auto sum = map.elevation_at(x, y) + map.elevation_at(x + square_size, y) +
              map.elevation_at(x + square_size, y + square_size) + map.elevation_at(x, y + square_size);

            auto value = sum / 4.0f + utility::random_number(dist);

            map.set_elevation_at(x + half_square, y + half_square, utility::clamp(value, 0.0f, max_elevation));
          }
        }

        for (std::int32_t y = 0; y <= map_size; y += square_size)
        {
          for (std::int32_t x = 0; x <= map_size; x += square_size)
          {
            auto do_square = [&](auto x, auto y)
            {
              std::pair<std::int32_t, std::int32_t> corners[] =
              {
                { x, y },
                { x + half_square, y - half_square },
                { x + square_size, y },
                { x + half_square, y + half_square }
              };

              auto sum = 0.0f;
              auto n = 0;
              for (auto corner : corners)
              {
                if (corner.first < 0 || corner.first > map_size || 
                    corner.second < 0 || corner.second > map_size) continue;

                sum += map.elevation_at(corner.first, corner.second);
                ++n;
              }

              if (x + half_square <= map_size && y <= map_size)
              {
                auto value = (sum / n) + utility::random_number(dist);
                map.set_elevation_at(x + half_square, y, utility::clamp(value, 0.0f, max_elevation));
              }
            };

            do_square(x, y);
            do_square(x - half_square, y + half_square);
          }
        }
      }

      map.set_cell_offset((map.grid_size() * cell_size - minimum_size) / (cell_size * 2));
      return map;
    }

    float interpolate_elevation_at(const ElevationMap& elevation_map, Vector2f position)
    {
      // TODO: Rewrite this to handle out-of-bounds better
      auto cell_offset = elevation_map.cell_offset();
      auto grid_size = elevation_map.grid_size();

      auto real_cell = position / elevation_map.cell_size();    
      auto real_cell_size = static_cast<float>(elevation_map.cell_size());

      auto cell = vector2_cast<std::int32_t>(make_vector2(std::floor(real_cell.x), real_cell.y));
      auto cell_fractional = real_cell - cell;

      IntRect bounds(-cell_offset, grid_size - 1);

      auto clamped_cell = make_vector2(utility::clamp(cell.x, bounds.left, bounds.right()),
                                       utility::clamp(cell.y, bounds.top, bounds.bottom()));

      auto clamped_position = make_vector2(utility::clamp(position.x, bounds.left * real_cell_size,
                                                          bounds.right() * real_cell_size),
                                           utility::clamp(position.y, bounds.top * real_cell_size,
                                                          bounds.bottom() * real_cell_size));

      auto left = clamped_cell.x;
      auto top = clamped_cell.y;
      auto right = clamped_cell.x + 1;
      auto bottom = clamped_cell.y + 1;

      auto world_left = left * real_cell_size;      
      auto world_top = top * real_cell_size;
      auto world_right = right * real_cell_size;
      auto world_bottom = bottom * real_cell_size;

      const Vector2f world_points[4] =
      {
        { world_left, world_top },
        { world_left, world_bottom },
        { world_right, world_top },
        { world_right, world_bottom }
      };

      auto world_center = make_vector2((world_left + world_right) * 0.5f, (world_top + world_bottom) * 0.5f);

      if (cell.x < clamped_cell.x) cell_fractional.x = 0.0f;
      else if (cell.x > clamped_cell.x)
      {
        cell_fractional.x = 1.0f;
        --right;
      }

      if (cell.y < clamped_cell.y) cell_fractional.y = 0.0f;
      else if (cell.y > clamped_cell.y)
      {        
        cell_fractional.y = 1.0f;
        --bottom;
      }

      const Vector2i points[4] =
      {
        { left, top },
        { left, bottom },
        { right, top },
        { right, bottom }
      };

      const float elevation_samples[4] =
      {
        elevation_map.elevation_at(points[0]),
        elevation_map.elevation_at(points[1]),
        elevation_map.elevation_at(points[2]),
        elevation_map.elevation_at(points[3])
      };

      auto center_sample = std::accumulate(elevation_samples, std::end(elevation_samples), 0.0f) * 0.25f;      

      auto interpolate_z = [&](int a, int b)
      {
        auto real_cell_size = static_cast<float>(elevation_map.cell_size());

        // Find the equation of the triangle, then calculate the z position according to this equation.
        auto t1 = make_3d(world_points[a], elevation_samples[a]);
        auto t2 = make_3d(world_points[b], elevation_samples[b]);
        auto t3 = make_3d(world_center, center_sample);

        auto cp = cross_product(t1 - t2, t1 - t3);
        auto d = -dot_product(cp, t1);

        return -(clamped_position.x * cp.x + clamped_position.y * cp.y + d) / cp.z;        
      };

      bool orientation = std::abs(cell_fractional.y - 0.5f) < std::abs(cell_fractional.x - 0.5f);
      if (orientation)
      {
        if (cell_fractional.x < 0.5f) return interpolate_z(0, 1);
        else return interpolate_z(2, 3);
      }

      else
      {
        if (cell_fractional.y < 0.5f) return interpolate_z(0, 2);
        else return interpolate_z(1, 3);
      }
    }

    Vector3f interpolate_terrain_normal_at(const ElevationMap& elevation_map, Vector2f position)
    {
      return Vector3f(0.0f, 0.0f, 1.0f);
    }
  }
}