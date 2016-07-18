/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef HEIGHT_MAP_HPP_5192385
#define HEIGHT_MAP_HPP_5192385

#include <vector>
#include <cstdint>

#include "utility/vector2.hpp"

namespace ts
{
  namespace resources_3d
  {
    class HeightMap
    {
    public:
      HeightMap() = default;

      explicit HeightMap(Vector2u size, std::uint32_t cell_size)
        : size_(size),
          cell_size_(cell_size),
          data_(size.x * size.y)
      {}

      explicit HeightMap(Vector2u size, std::uint32_t cell_size, std::vector<float> data)
        : size_(size),
          cell_size_(cell_size),
          data_(std::move(data))
      {
        data_.resize(size_.x * size_.y);
      }

      Vector2u size() const
      {
        return size_;
      }

      void resize(Vector2u new_size)
      {
        size_ = new_size;

        // TODO: Resize algorithm
        data_.resize(size_.x * size_.y);
      }

      std::uint32_t cell_size() const
      {
        return cell_size_;
      }

      void set_cell_size(std::uint32_t cell_size)
      {
        cell_size_ = cell_size;
      }

      float& operator()(std::size_t x, std::size_t y)
      {
        return data_[y * size_.x + x];
      }

      const float& operator()(std::size_t x, std::size_t y) const
      {
        return data_[y * size_.x + x];
      }

    private:
      Vector2u size_ = { 0, 0 };
      std::uint32_t cell_size_ = 16;
      std::vector<float> data_;
    };


    HeightMap generate_height_map(std::uint32_t map_size, std::uint32_t cell_size,
                                  float max_z, float variance = 1.0f);

    float interpolate_height_at(const HeightMap& height_map, Vector2f position);
  }
}

#endif