/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "track_3d.hpp"

namespace ts
{
  namespace resources3d
  {
    Track::Track(Vector2i size)
      : size_(size),
        elevation_map_((size + 15) / 16, 16, 0.0f)
    {
    }

    Track::Track(Vector2i size, ElevationMap elevation_map)
      : size_(size),
        elevation_map_(std::move(elevation_map))
    {}

    Vector2i Track::size() const
    {
      return size_;
    }

    void Track::resize(Vector2i new_size)
    {
      size_ = new_size;
      // TODO: Resize elevation map if it's too small.
    }

    const ElevationMap& Track::elevation_map() const
    {
      return elevation_map_;
    }

    void Track::assign_elevation_map(ElevationMap map)
    {
      // TODO: Resize map if it's too small.
      elevation_map_ = std::move(map);
    }

    const TextureLibrary& Track::texture_library() const
    {
      return texture_library_;
    }

    TextureLibrary& Track::texture_library()
    {
      return texture_library_;
    }

    PathLayer* Track::create_path_layer()
    {
      std::uint32_t layer_id = 0;
      if (!path_layers_.empty())
      {
        layer_id = path_layers_.rbegin()->first + 1;
      }

      auto path_layer = &path_layers_[layer_id];
      path_layer_order_.push_back(path_layer);
      return path_layer;
    }

    boost::iterator_range<const PathLayer* const*> Track::path_layers() const
    {
      return{ path_layer_order_.data(), path_layer_order_.data() + path_layer_order_.size() };
    }
  }
}