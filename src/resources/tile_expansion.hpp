/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef TILE_EXPANSION_5829322
#define TILE_EXPANSION_5829322

#include "tile_library.hpp"

#include "utility/transform.hpp"
#include "utility/vector2.hpp"

#include <iostream>

namespace ts
{
  namespace resources
  {
    // This algorithm can be used to transform a range of Tile objects and a TileLibrary into
    // an output range of PlacedTile objects. Tile groups are expanded in the process,
    // taking the sub-tiles and calculating their positions as if they were normal tiles.

    template <typename InputIt, typename OutIt>
    void expand_tiles(InputIt tile_it, InputIt tile_end, std::uint32_t level, 
                      const TileLibrary& tile_library, OutIt out)
    {
      const auto& tile_groups = tile_library.tile_groups();
      const auto& tiles = tile_library.tiles();

      // For every tile...
      for (; tile_it != tile_end; ++tile_it)
      {
        const Tile& tile = *tile_it;
        auto tile_group = tile_groups.find(tile.id);
        if (tile_group == tile_groups.end())
        {
          auto tile_def_it = tiles.find(tile.id);
          if (tile_def_it != tiles.end())
          {
            // It's not a tile group but a regular tile, so this will just be a simple copy.
            PlacedTile placed_tile;
            placed_tile.id = tile.id;
            placed_tile.level = level + tile.level;
            placed_tile.position = tile.position;
            placed_tile.rotation = tile.rotation;
            placed_tile.definition = &*tile_def_it;

            *out = placed_tile; 
            ++out;
          }
        }

        else
        {
          // It is a tile group, so we have to loop through all sub-tiles, transform
          // their positions and write the resulting tile to the output buffer.
          for (const auto& sub_tile : tile_group->sub_tiles)
          {
            auto tile_def_it = tiles.find(sub_tile.id);
            if (tile_def_it == tiles.end()) continue;

            auto tile_rotation = degrees(static_cast<double>(tile.rotation));

            auto sub_tile_position = vector2_cast<double>(sub_tile.position);
            auto sub_tile_offset = transform_point(sub_tile_position, tile_rotation);

            PlacedTile placed_tile;
            placed_tile.id = sub_tile.id;
            placed_tile.level = sub_tile.level;
            placed_tile.position = tile.position + vector2_round<std::int32_t>(sub_tile_offset);
            placed_tile.definition = &*tile_def_it;
            placed_tile.rotation = tile.rotation + sub_tile.rotation;

            *out = placed_tile;
            ++out;
          }
        }
      }
    }
  }
}

#endif