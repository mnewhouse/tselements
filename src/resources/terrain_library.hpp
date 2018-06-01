/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "terrain_definition.hpp"

#include <vector>
#include <cstdint>

namespace ts
{
  namespace resources
  {
    // The TerrainLibrary represents, well, a library of terrains.
    // It exposes a fixed-size array of regular terrains, which define various properties 
    // that affect the handling.
    class TerrainLibrary
    {
    public:
      void define_terrain(const TerrainDefinition& terrain);
      const TerrainDefinition& terrain(TerrainId terrain_id) const;

      static constexpr std::uint32_t max_terrains = 256;

    private:
      std::vector<TerrainDefinition> terrains_ = std::vector<TerrainDefinition>(max_terrains);
    };
  }
}