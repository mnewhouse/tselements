/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef TERRAIN_DEFINITION_HPP_581918291823
#define TERRAIN_DEFINITION_HPP_581918291823

#include "utility/color.hpp"

#include <cstdint>

namespace ts
{
  namespace resources
  {
    using TerrainId = std::uint8_t;

    struct TerrainDefinition
    {
      TerrainId id;

      double acceleration = 1.0;
      double braking = 1.0;
      double steering = 1.0;
      double grip = 1.0;
      double traction = 1.0;
      double roughness = 0.0;
      double jump = 0.0;
      double bounciness = 1.0;

      Colorb color = Colorb{ 150, 150, 150, 255 };

      bool tyre_mark = false;
      bool skid_mark = true;
      bool is_wall = false;
    };

    struct SubTerrain
    {
      TerrainId sub_terrain;
      std::uint8_t level_start;
      std::uint8_t num_levels;
    };
  }
}

#endif