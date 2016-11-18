/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "utility/rect.hpp"
#include "utility/vector2.hpp"

#include <cstdint>
#include <vector>
#include <string>

#include <boost/container/small_vector.hpp>
#include <boost/utility/string_ref.hpp>

namespace ts
{
  namespace resources
  {
    using TileId = std::uint16_t;
    static constexpr TileId max_tile_id = 8192;

    struct TileDefinition
    {
      TileId id;
      boost::string_ref pattern_file;
      boost::string_ref image_file;
      IntRect pattern_rect;
      IntRect image_rect;
    };

    struct Tile
    {
      TileId id;
      Vector2i position;
      std::int32_t rotation;
      std::uint32_t level;
    };

    struct PlacedTile
      : Tile
    {
      const TileDefinition* definition;
    };

    struct TileGroupDefinition
    {
      TileId id;
      boost::container::small_vector<Tile, 8> sub_tiles;
    };
  }
}
