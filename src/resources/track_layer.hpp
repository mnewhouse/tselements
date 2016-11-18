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
    enum class TrackLayerType
    {
      Tiles, Geometry, Paths
    };

    struct TrackLayer
    {
      using Id = std::uint32_t;

      Id id;
      TrackLayerType type;
      std::uint32_t level;
      std::string name;

      std::vector<Tile> tiles;
      std::vector<Geometry> geometry;      
      std::vector<TrackPath> paths;
    };
  }
}
