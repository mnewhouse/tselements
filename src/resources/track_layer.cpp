/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "track_layer.hpp"

namespace ts
{
  namespace resources
  {
    TrackLayer::TrackLayer(TrackLayerType type, std::uint32_t level, std::string name)
      : type_(type),
        level_(level),
        name_(std::move(name))
    {}

    void TrackLayer::set_level(std::uint32_t level)
    {
      level_ = level;
    }

    void TrackLayer::set_z_index(std::uint32_t z_index)
    {
      z_index_ = z_index;
    }

    void TrackLayer::set_visible(bool show)
    {
      visible_ = show;
    }

    void TrackLayer::rename(const std::string& new_name)
    {
      name_ = new_name;
    }

    std::vector<Tile>& TrackLayer::tiles()
    {
      return tiles_;
    }

    const std::vector<Tile>& TrackLayer::tiles() const
    {
      return tiles_;
    }

    std::vector<Geometry>& TrackLayer::geometry()
    {
      return geometry_;
    }

    const std::vector<Geometry>& TrackLayer::geometry() const
    {
      return geometry_;
    }

    std::vector<TrackPath>& TrackLayer::paths()
    {
      return paths_;
    }

    const std::vector<TrackPath>& TrackLayer::paths() const
    {
      return paths_;
    }
  }
}