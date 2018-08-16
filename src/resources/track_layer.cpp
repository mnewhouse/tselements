/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
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
    {
      if (type_ == TrackLayerType::Tiles) data_ = std::vector<Tile>();
      else if (type_ == TrackLayerType::PathStyle) data_ = PathLayerData();
      else if (type_ == TrackLayerType::BaseTerrain) data_ = BaseTerrainData();
    }

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

    TrackLayer::data_type& TrackLayer::data()
    {
      return data_;
    }

    const TrackLayer::data_type& TrackLayer::data() const
    {
      return data_;
    }

    std::vector<Tile>* TrackLayer::tiles()
    {
      return boost::get<std::vector<Tile>>(&data_);
    }

    const std::vector<Tile>* TrackLayer::tiles() const
    {
      return boost::get<std::vector<Tile>>(&data_);
    }

    const PathLayerData* TrackLayer::path_style() const
    {
      return boost::get<PathLayerData>(&data_);
    }

    PathLayerData* TrackLayer::path_style()
    {
      return boost::get<PathLayerData>(&data_);
    }

    BaseTerrainData* TrackLayer::base_terrain()
    {
      return boost::get<BaseTerrainData>(&data_);
    }

    const BaseTerrainData* TrackLayer::base_terrain() const
    {
      return boost::get<BaseTerrainData>(&data_);
    }
  }
}