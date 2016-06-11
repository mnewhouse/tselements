/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

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
      const std::size_t row_width = map_size + 1;

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
      auto coords = position / static_cast<float>(height_map.cell_size());

      auto map_size = height_map.size();

      auto x = std::min(static_cast<std::uint32_t>(coords.x), map_size.x - 1);
      auto y = std::min(static_cast<std::uint32_t>(coords.y), map_size.y - 1);      

      auto right_weight = coords.x - std::floor(coords.x);
      auto bottom_weight = coords.y - std::floor(coords.y);
      auto left_weight = 1.0f - right_weight;
      auto top_weight = 1.0f - bottom_weight;
        
      auto right = x != map_size.x - 1 ? x : x + 1;
      auto bottom = y != map_size.y - 1 ? y : y + 1;

      return height_map(x, y) * left_weight * top_weight +
        height_map(right, y) * right_weight * top_weight +
        height_map(right, bottom) * right_weight * bottom_weight +
        height_map(x, bottom) * left_weight * bottom_weight;
    }
  }
}