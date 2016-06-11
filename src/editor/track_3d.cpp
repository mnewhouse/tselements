/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "track_3d.hpp"

namespace ts
{
  namespace resources_3d
  {
    Vector3u Track::size() const
    {
      return size_;
    }

    Vector2u Track::size_2d() const
    {
      return{ size_.x, size_.y };
    }

    void Track::resize(Vector3u new_size)
    {
      size_ = new_size;

      auto map_size = height_map_.size();
      auto cell_size = height_map_.cell_size();

      // If there are not enough cells in the height map to represent the entire track dsize...
      if (size_.x > map_size.x * cell_size || size_.y > map_size.y * cell_size)
      {
        map_size.x = (size_.x + cell_size + 1) / cell_size;
        map_size.y = (size_.y + cell_size + 1) / cell_size;

        height_map_.resize(map_size);
      }
    }

    const TextureLibrary& Track::texture_library() const
    {
      return texture_library_;
    }

    void Track::update_height_map(HeightMap height_map)
    {
      height_map_ = std::move(height_map);

      auto cell_size = height_map_.cell_size();
      auto map_size = height_map_.size() * cell_size;
      if (size_.x > map_size.x || size_.y > map_size.y)
      {
        map_size.x = (size_.x + 1 + cell_size) / cell_size;
        map_size.y = (size_.y + 1 + cell_size) / cell_size;

        height_map_.resize(map_size);
      }
    }

    const HeightMap& Track::height_map() const
    {
      return height_map_;
    }

    void Track::define_texture(std::uint16_t id, std::string image_file, IntRect image_rect)
    {
      texture_library_.define_texture(id, std::move(image_file), image_rect);
    }

    std::uint16_t Track::base_texture() const
    {
      return base_texture_;
    }

    void Track::set_base_texture(std::uint16_t texture_id)
    {
      base_texture_ = texture_id;
    }

    TrackPath* Track::create_path()
    {
      std::size_t id = 0;
      if (!track_paths_.empty()) id = track_paths_.rbegin()->first + 1;

      auto path = &track_paths_[id];
      active_paths_.push_back(path);
      
      return path;
    }

    Track::path_range Track::paths()
    {
      return path_range(active_paths_.data(), active_paths_.data() + active_paths_.size());
    }

    Track::const_path_range Track::paths() const
    {
      return const_path_range(active_paths_.data(), active_paths_.data() + active_paths_.size());
    }
  }
}