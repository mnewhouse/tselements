/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

namespace ts
{
  namespace resources
  {
    class Track;
  }

  namespace world
  {
    class TerrainMap;

    TerrainMap build_terrain_map(const resources::Track& track);
  }
}