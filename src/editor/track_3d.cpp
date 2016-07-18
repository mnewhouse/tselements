/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"

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

      ensure_height_map_size();
    }

    const TextureLibrary& Track::texture_library() const
    {
      return texture_library_;
    }

    void Track::ensure_height_map_size()
    {
      auto map_size = height_map_.size();
      auto cell_size = height_map_.cell_size();
      if (map_size.x * cell_size < size_.x || map_size.y * cell_size < size_.y)
      {
        height_map_.resize(Vector2u((size_.x + cell_size - 1) / cell_size,
                                    (size_.y + cell_size - 1) / cell_size));
      }
    }

    void Track::adopt_height_map(HeightMap height_map)
    {
      height_map_ = std::move(height_map);

      ensure_height_map_size();
    }

    const HeightMap& Track::height_map() const
    {
      return height_map_;
    }

    void Track::raise_elevation_at(Vector2u map_coord, float amount)
    {
      auto new_z = height_map_(map_coord.x, map_coord.y) + amount;
      set_elevation_at(map_coord, new_z);
    }

    void Track::set_elevation_at(Vector2u map_coord, float z)
    {
      auto new_z = std::max(std::min(z, static_cast<float>(size_.z)), 0.0f);
      height_map_(map_coord.x, map_coord.y) = new_z;
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