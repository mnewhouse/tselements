/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"

#include "height_map.hpp"

#include "utility/random.hpp"

#include <random>

namespace ts
{
  namespace resources_3d
  {
    HeightMap generate_height_map(std::uint32_t map_size, std::uint32_t cell_size, 
                                  float max_z, float variance)
    {
      const std::uint32_t row_width = map_size + 1;

      HeightMap map(Vector2u(row_width, row_width), cell_size);

      {
        std::uniform_real_distribution<float> dist(0.0, max_z);       

        map(0, 0) = utility::random_number(dist);
        map(map_size, 0) = utility::random_number(dist);
        map(map_size, map_size) = utility::random_number(dist);
        map(0, map_size) = utility::random_number(dist);
      }

      variance *= max_z * 0.5f;
      for (auto square_size = map_size; square_size > 1; square_size /= 2, variance *= 0.5f)
      {
        const auto half_square = square_size / 2;
        std::uniform_real_distribution<float> dist(-variance, variance);

        for (std::size_t y = 0; y < map_size; y += square_size)
        {
          for (std::size_t x = 0; x < map_size; x += square_size)
          {
            auto sum = map(x, y) + map(x + square_size, y) +
              map(x + square_size, y + square_size) + map(x, y + square_size);

            auto value = sum / 4.0f + utility::random_number(dist);

            map(x + half_square, y + half_square) = std::min(std::max(value, 0.0f), max_z);
          }
        }

        for (std::size_t y = 0; y <= map_size; y += square_size)
        {
          for (std::size_t x = 0; x <= map_size; x += square_size)
          {
            auto do_square = [&](std::size_t x, std::size_t y)
            {
              std::pair<std::size_t, std::size_t> corners[] =
              {
                { x, y },
                { x + half_square, y - half_square },
                { x + square_size, y },
                { x + half_square, y + half_square }
              };

              auto sum = 0.0f;
              std::size_t n = 0;
              for (auto corner : corners)
              {
                if (corner.first > map_size || corner.second > map_size) continue;

                sum += map(corner.first, corner.second);
                ++n;
              }


              if (x + half_square <= map_size && y <= map_size)
              {
                auto value = (sum / n) + utility::random_number(dist);
                map(x + half_square, y) = std::min(std::max(value, 0.0f), max_z);
              }
            };

            do_square(x, y);
            do_square(x - half_square, y + half_square);
          }
        }
      }

      return map;
    }

    float interpolate_height_at(const HeightMap& height_map, Vector2f position)
    {
      auto cell_size = height_map.cell_size();
      auto map_size = height_map.size();

      auto real_cell_size = static_cast<float>(cell_size);
      auto coords = position / real_cell_size;      

      std::uint32_t x = 0;
      std::uint32_t y = 0;

      if (coords.x >= 0.0f) x = std::min(static_cast<std::uint32_t>(coords.x), map_size.x - 1);
      if (coords.y >= 0.0f) y = std::min(static_cast<std::uint32_t>(coords.y), map_size.y - 1);

      auto offset = make_vector2(coords.x - std::floor(coords.x), coords.y - std::floor(coords.y));

      auto right = (x == map_size.x - 1 ? x : x + 1);
      auto bottom = (y == map_size.y - 1 ? y : y + 1);

      float weights[3] =
      {
        1.0f - offset.x,
        offset.y,
        offset.x - offset.y
      };

      std::pair<std::uint32_t, std::uint32_t> points[3] =
      {
        { x, y },
        { right, bottom },
        { right, y }
      };

      if (offset.y >= offset.x)
      {
        weights[2] = offset.y - offset.x;
        points[2] = { x, bottom };
      }     

      auto total_height = 0.0f;
      auto total_weight = 0.0f;
      auto idx = 0;
      for (const auto& point : points)
      {
        total_height += height_map(point.first, point.second) * weights[idx];
        total_weight += weights[idx];

        ++idx;
      }

      return total_height / total_weight;
    }
  }
}