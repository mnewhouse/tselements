/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/


#include "terrain_library.hpp"

#include <algorithm>

namespace ts
{
  namespace resources
  {
    void TerrainLibrary::define_terrain(const TerrainDefinition& terrain)
    {
      terrains_[terrain.id] = terrain;
      
      // Make the terrain reference itself on level 0.
      SubTerrain sub_terrain;
      sub_terrain.level_start = 0;
      sub_terrain.num_levels = 1;
      sub_terrain.sub_terrain = terrain.id;
      define_sub_terrain(terrain.id, sub_terrain);
    }

    const TerrainDefinition& TerrainLibrary::terrain(TerrainId terrain_id) const
    {
      return terrains_[terrain_id];
    }

    const TerrainDefinition& TerrainLibrary::terrain(TerrainId terrain_id, std::uint8_t level) const
    {
      return terrain(sub_terrain(terrain_id, level));
    }

    void TerrainLibrary::define_sub_terrain(TerrainId terrain_id, SubTerrain sub_terrain)
    {
      if (sub_terrain.num_levels != 0)
      {
        // Calculate the pointer, do some bounds checking, and just fill
        // with the referenced sub-terrain.
        auto level_range = std::make_pair(sub_terrain.level_start,
                                          sub_terrain.level_start + sub_terrain.num_levels);
        if (level_range.second >= max_levels) level_range.second = max_levels;

        auto block = sub_terrains_.data() + terrain_id * max_levels;
        auto end = block + level_range.second;

        std::fill(block + sub_terrain.level_start, end, sub_terrain.sub_terrain);

        auto level_block = sub_levels_.data() + terrain_id * max_levels;
        auto level_end = level_block + (end - block);

        --level_range.second;
        std::fill(level_block + sub_terrain.level_start, level_end, level_range);
      }
    }

    TerrainId TerrainLibrary::sub_terrain(TerrainId terrain_id, std::uint8_t level) const
    {
      return sub_terrains_[terrain_id * max_levels + level];
    }

    TerrainLibrary::level_range_type 
      TerrainLibrary::sub_level_range(TerrainId terrain_id, std::uint8_t level) const
    {
      return sub_levels_[terrain_id * max_levels + level];
    }
  }
}