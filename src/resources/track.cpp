/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "track.hpp"
#include "default_start_points.hpp"

#include <algorithm>

namespace ts
{
  namespace resources
  {
    struct LayerOrderComparator
    {
      bool operator()(const TrackLayer* a, const TrackLayer* b) const noexcept
      {
        return a->level < b->level;
      }
    };

    void Track::set_path(const std::string& path)
    {
      path_ = path;
    }
    
    const std::string& Track::path() const noexcept
    {
      return path_;
    }

    void Track::set_author(std::string author)
    {
      author_ = std::move(author);
    }

    const std::string& Track::author() const noexcept
    {
      return author_;
    }

    Vector2u Track::size() const
    {
      return size_;
    }

    void Track::set_size(const Vector2u& size)
    {
      size_ = size;
    }

    std::uint32_t Track::height_level_count() const noexcept
    {
      return height_level_count_;
    }

    void Track::set_height_level_count(std::uint32_t height_levels) noexcept
    {
      height_level_count_ = height_levels;
    }    

    TileLibrary& Track::tile_library() noexcept
    {
      return tile_library_;
    }

    const TileLibrary& Track::tile_library() const noexcept
    {
      return tile_library_;
    }

    TerrainLibrary& Track::terrain_library() noexcept
    {
      return terrain_library_;
    }

    const TerrainLibrary& Track::terrain_library() const noexcept
    {
      return terrain_library_;
    }

    TextureLibrary& Track::texture_library() noexcept
    {
      return texture_library_;
    }

    const TextureLibrary& Track::texture_library() const noexcept
    {
      return texture_library_;
    }

    const TrackLayer* Track::get_layer_by_id(LayerId layer_id) const
    {
      auto it = layers_.find(layer_id);
      if (it == layers_.end()) return nullptr;

      return &it->second;
    }

    TrackLayer* Track::get_layer_by_id(LayerId layer_id)
    {
      auto it = layers_.find(layer_id);
      if (it == layers_.end()) return nullptr;

      return &it->second;      
    }

    Track::LayerOrderInterface Track::layers()
    {
      TrackLayer* const* begin = layer_order_.data();
      TrackLayer* const* end = begin + layer_order_.size();

      return LayerOrderInterface(boost::make_indirect_iterator(begin), boost::make_indirect_iterator(end));
    }

    Track::ImmutableLayerOrderInterface Track::layers() const
    {
      const TrackLayer* const* begin = layer_order_.data();
      const TrackLayer* const* end = begin + layer_order_.size();

      return ImmutableLayerOrderInterface(boost::make_indirect_iterator(begin), boost::make_indirect_iterator(end));
    }

    TrackLayer* Track::create_layer(std::string layer_name, std::uint32_t level)
    {
      LayerId layer_id = 0;
      if (!layers_.empty()) layer_id = layers_.crbegin()->first + 1;

      TrackLayer layer;
      layer.id = layer_id;
      layer.name = std::move(layer_name);
      layer.level = level;

      auto result = layers_.insert(std::make_pair(layer_id, layer));
      auto layer_ptr = &result.first->second;
      
      // Make sure to keep the layers sorted by level
      auto& order = layer_order_;
      auto insert_position = std::upper_bound(order.begin(), order.end(), layer_ptr, LayerOrderComparator());
      order.insert(insert_position, layer_ptr);

      return layer_ptr;
    }

    std::size_t Track::layer_count() const noexcept
    {
      return layers_.size();
    }

    void Track::add_control_point(const ControlPoint& point)
    {
      if (control_points_.empty())
      {
        // If we are adding a finish line, make sure the start point cache gets invalidated.
        start_points_.clear();
      }

      control_points_.push_back(point);
    }

    const std::vector<ControlPoint>& Track::control_points() const
    {
      return control_points_;
    }

    void Track::add_start_point(const StartPoint& point)
    {
      start_points_.clear();
      custom_start_points_.push_back(point);
    }

    const std::vector<StartPoint>& Track::start_points() const
    {      
      if (!custom_start_points_.empty())
      {  
        // If we have any custom start points, return those
        return custom_start_points_;
      }

      else if (start_points_.empty() && !control_points_.empty())
      {
        // We have a finish line and no custom start points, 
        // so we can generate the default start points.

        generate_default_start_points(control_points_.front(), 20, 12, 
                                      std::back_inserter(start_points_));
      }

      return start_points_;     
    }
  }
}