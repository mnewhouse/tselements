/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "tiles.hpp"
#include "geometry.hpp"
#include "track_path.hpp"

#include <vector>

namespace ts
{
  namespace resources
  {
    class Track;

    enum class TrackLayerType
    {
      Tiles, Geometry, Paths
    };

    class TrackLayer
    {
    public:
      explicit TrackLayer(TrackLayerType type, std::uint32_t level, std::string name);      

      std::uint32_t level() const { return level_; }
      TrackLayerType type() const { return type_; }
      bool visible() const { return visible_; }
      const std::string& name() const { return name_; }

      void set_level(std::uint32_t level);
      void set_visible(bool show);

      void hide() { set_visible(false); }
      void show() { set_visible(true); }

      void rename(const std::string& new_name);

      std::vector<Tile>& tiles();
      const std::vector<Tile>& tiles() const;

      std::vector<Geometry>& geometry();
      const std::vector<Geometry>& geometry() const;
      
      std::vector<TrackPath>& paths();
      const std::vector<TrackPath>& paths() const;

    private:
      TrackLayerType type_;
      std::uint32_t level_ = 0;
      
      bool visible_ = true;
      std::string name_;

      std::vector<Tile> tiles_;
      std::vector<Geometry> geometry_;      
      std::vector<TrackPath> paths_;     
    };
  }
}
