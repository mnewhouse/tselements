/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "tiles.hpp"
#include "geometry.hpp"
#include "track_path.hpp"

#include "utility/color.hpp"

#include <vector>

#include <boost/variant.hpp>

namespace ts
{
  namespace resources
  {
    class Track;

    enum class TrackLayerType
    {
      Tiles, BaseTerrain, Geometry, PathStyle
    };

    struct PathLayerData
    {
      const resources::TrackPath* path;
      std::vector<PathStyle> styles;
    };

    struct BaseTerrainData
    {
      std::uint32_t texture_id;
      std::uint32_t terrain_id;
      Colorb color = Colorb(255, 255, 255, 255);
    };

    class TrackLayer
    {
    public:
      explicit TrackLayer(TrackLayerType type, std::uint32_t level, std::string name); // Tile layer by default

      std::uint32_t level() const { return level_; }
      std::uint32_t z_index() const { return z_index_; };
      TrackLayerType type() const { return type_; }
      bool visible() const { return visible_; }
      const std::string& name() const { return name_; }

      void set_visible(bool show);

      void hide() { set_visible(false); }
      void show() { set_visible(true); }

      void rename(const std::string& new_name);

      using data_type = boost::variant<std::vector<Tile>, PathLayerData, BaseTerrainData>;
      const data_type& data() const;
      data_type& data();

      const std::vector<Tile>* tiles() const;
      std::vector<Tile>* tiles();

      const PathLayerData* path_styles() const;
      PathLayerData* path_styles();      

      BaseTerrainData* base_terrain();
      const BaseTerrainData* base_terrain() const;

    private:
      friend Track;
      void set_level(std::uint32_t level);
      void set_z_index(std::uint32_t z_index);

      TrackLayerType type_;
      std::uint32_t level_ = 0;
      std::uint32_t z_index_ = 0;
      
      bool visible_ = true;
      std::string name_;
      
      data_type data_;
    };
  }
}
