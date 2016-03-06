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
    void TerrainLibrary::define_terrain(TerrainId terrain_id, const TerrainDefinition& terrain)
    {
      terrains_[terrain_id] = terrain;
      
      // Make the terrain reference itself on level 0.
      SubTerrain sub_terrain;
      sub_terrain.level_start = 0;
      sub_terrain.num_levels = max_levels;
      sub_terrain.sub_terrain = terrain_id;
      define_sub_terrain(terrain_id, sub_terrain);
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
      // Calculate the pointer, do some bounds checking, and just fill
      // with the referenced sub-terrain.
      auto block = sub_terrains_.data() + terrain_id * max_levels;
      auto end = std::min(block + sub_terrain.level_start + sub_terrain.num_levels, block + max_levels);

      std::fill(block + sub_terrain.level_start, end, sub_terrain.sub_terrain);
    }

    TerrainId TerrainLibrary::sub_terrain(TerrainId terrain_id, std::uint8_t level) const
    {
      return sub_terrains_[terrain_id * max_levels + level];
    }
  }
}