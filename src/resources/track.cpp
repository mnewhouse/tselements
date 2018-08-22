/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
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
      bool operator()(const TrackLayer* a, const TrackLayer* b) const
      {
        return a->level() < b->level();
      }
    };

    void Track::add_asset(const std::string& asset)
    {
      assets_.push_back(asset);
    }

    const std::vector<std::string>& Track::assets() const
    {
      return assets_;
    }

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

    Vector2i Track::size() const
    {
      return size_;
    }

    void Track::set_size(const Vector2i& size)
    {
      size_ = size;
    }

    std::int32_t Track::height_level_count() const noexcept
    {
      return height_level_count_;
    }

    void Track::set_height_level_count(std::int32_t height_levels) noexcept
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

    PathLibrary& Track::path_library() noexcept
    {
      return path_library_;
    }

    const PathLibrary& Track::path_library() const noexcept
    {
      return path_library_;
    }

    Track::LayerOrderInterface Track::layers()
    {
      TrackLayer* const* begin = layer_order_.data();
      TrackLayer* const* end = begin + layer_order_.size();

      return LayerOrderInterface(boost::make_indirect_iterator(begin), boost::make_indirect_iterator(end));
    }

    Track::ConstLayerOrderInterface Track::layers() const
    {
      const TrackLayer* const* begin = layer_order_.data();
      const TrackLayer* const* end = begin + layer_order_.size();

      return ConstLayerOrderInterface(boost::make_indirect_iterator(begin), boost::make_indirect_iterator(end));
    }

    void Track::set_layer_level(TrackLayer* layer, std::uint32_t level)
    {
      auto layer_it = std::find(layer_order_.begin(), layer_order_.end(), layer);      

      if (layer_it != layer_order_.end())
      {
        // Shift the layer to the closest spot that would keep everything sorted

        auto old_level = layer->level();
        auto predicate = [=](const TrackLayer* layer)
        {
          return layer->level() >= level;
        };

        if (level < old_level)
        {
          // Find the first layer that has a level greater than or equal to our new level
          auto range_end = std::find_if(std::next(layer_it), layer_order_.end(), predicate);
          std::rotate(layer_it, std::next(layer_it), range_end);
        }

        else if (level > old_level)
        {
          auto rev = std::make_reverse_iterator(layer_it);
          auto range_end = std::find_if(rev, layer_order_.rend(), predicate);
          std::rotate(std::prev(rev), range_end, rev);
        }

        layer->set_level(level);

        std::uint32_t z_index = 0;
        for (auto layer : layer_order_) layer->set_z_index(z_index++);
      }      
    }

    std::int32_t Track::shift_towards_front(const TrackLayer* layer, std::int32_t shift_amount)
    {
      auto layer_it = std::find(layer_order_.begin(), layer_order_.end(), layer);
      if (layer_it == layer_order_.end()) return 0;

      auto level = layer->level();
      auto next = std::next(layer_it);

      std::int32_t i = 0;
      while (i < shift_amount && next != layer_order_.end() && (*next)->level() == level)
      {
        std::iter_swap(layer_it++, next++);

        ++i;
      }

      std::uint32_t z_index = 0;
      for (auto layer : layer_order_) layer->set_z_index(z_index++);

      return i;
    }

    std::int32_t Track::shift_towards_back(const TrackLayer* layer, std::int32_t shift_amount)
    {
      auto layer_it = std::find(layer_order_.begin(), layer_order_.end(), layer);
      if (layer_it == layer_order_.end() || layer_it == layer_order_.begin()) return 0;

      auto level = layer->level();
      auto prev = std::prev(layer_it);

      std::int32_t i = 0;
      while (i < shift_amount && (*prev)->level() == level)
      {
        // This loop is a bit tricky, but this is the best way to write it that I could think of.
        std::iter_swap(layer_it, prev);
        ++i;

        if (prev == layer_order_.begin()) break;

        --layer_it; 
        --prev;        
      }

      std::uint32_t z_index = 0;
      for (auto layer : layer_order_) layer->set_z_index(z_index++);

      return i;
    }

    TrackLayer* Track::create_layer(TrackLayerType type, std::string layer_name, std::uint32_t level)                                    
    {
      LayerId layer_id = 0;
      if (!layers_.empty()) layer_id = layers_.crbegin()->first + 1;

      auto result = layers_.emplace(std::piecewise_construct, std::forward_as_tuple(layer_id),
                                    std::forward_as_tuple(type, level, std::move(layer_name)));

      auto layer_ptr = &result.first->second;
      
      // Make sure to keep the layers sorted by level
      auto& order = layer_order_;
      auto insert_position = std::upper_bound(order.begin(), order.end(), layer_ptr, LayerOrderComparator());
      order.insert(insert_position, layer_ptr);

      std::uint32_t z_index = 0;
      for (auto layer : layer_order_) layer->set_z_index(z_index++);

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

    void Track::add_control_point(const ControlPoint& point, std::size_t idx)
    {
      if (control_points_.empty())
      {
        start_points_.clear();
      }

      idx = std::min(control_points_.size(), idx);
      control_points_.insert(control_points_.begin() + idx, point);
    }

    void Track::remove_control_point(std::size_t idx)
    {
      if (idx < control_points_.size())
      {
        if (idx == 0)
        {
          start_points_.clear();
        }

        control_points_.erase(control_points_.begin() + idx);
      }
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

    const std::vector<StartPoint>& Track::custom_start_points() const
    {
      return custom_start_points_;
    }
  }
}