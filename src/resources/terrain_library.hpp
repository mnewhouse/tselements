/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef TERRAIN_LIBRARY_HPP_43189182985
#define TERRAIN_LIBRARY_HPP_43189182985

#include "terrain_definition.hpp"

#include <vector>
#include <cstdint>

namespace ts
{
  namespace resources
  {
    // The TerrainLibrary represents, well, a library of terrains.
    // It exposes a fixed-size array of regular terrains, which define various properties 
    // that affect the handling. Sub-terrains can be defined for every terrain at every height level,
    // which are basically just references to regular terrains.
    class TerrainLibrary
    {
    public:
      void define_terrain(TerrainId terrain_id, const TerrainDefinition& terrain);
      const TerrainDefinition& terrain(TerrainId terrain_id) const;
      const TerrainDefinition& terrain(TerrainId terrain_id, std::uint8_t level) const;

      void define_sub_terrain(TerrainId terrain_id, SubTerrain sub_terrain);
      TerrainId sub_terrain(TerrainId terrain_id, std::uint8_t level) const;

      static constexpr std::uint32_t max_terrains = 256;
      static constexpr std::uint32_t max_levels = 16;

    private:
      std::vector<TerrainDefinition> terrains_ = std::vector<TerrainDefinition>(max_terrains);
      std::vector<TerrainId> sub_terrains_ = std::vector<TerrainId>(max_terrains * max_levels);
    };
  }
}

#endif