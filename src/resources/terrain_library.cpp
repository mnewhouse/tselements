/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
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
    }

    const TerrainDefinition& TerrainLibrary::terrain(TerrainId terrain_id) const
    {
      return terrains_[terrain_id];
    }
  }
}