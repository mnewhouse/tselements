/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "model_3d.hpp"

#include "utility/vector2.hpp"
#include "utility/color.hpp"
#include "utility/rect.hpp"
#include "utility/triangle_utilities.hpp"

#include "graphics/texture.hpp"

#include <utility>
#include <cstdint>
#include <cstddef>
#include <array>
#include <vector>

namespace ts
{
  namespace resources3d
  {
    class ElevationMap;

    struct TerrainModel
    {
      struct Component
      {
        std::uint32_t face_index;
        std::uint32_t face_count;
        std::size_t model_tag;
      };

      std::vector<Vertex> vertices;
      std::vector<Face> faces;
      std::vector<Component> components;
    };
    
    class TerrainBuilder
    {
    public:

    };
  }
}