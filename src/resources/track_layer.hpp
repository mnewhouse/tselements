/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef TRACK_LAYER_HPP_544189
#define TRACK_LAYER_HPP_544189

#include "tiles.hpp"
#include "track_geometry.hpp"

#include <vector>

namespace ts
{
  namespace resources
  {
    struct TrackLayer
    {
      using Id = std::uint32_t;

      Id id;
      std::uint32_t level;
      std::string name;
      std::vector<TrackGeometry> geometry;
      std::vector<Tile> tiles;
    };
  }
}

#endif