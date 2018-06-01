/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "track_texture.hpp"
#include "terrain_definition.hpp"

#include "utility/vector2.hpp"
#include "utility/color.hpp"

#include <cstdint>
#include <vector>
#include <array>

namespace ts
{
  namespace resources
  {
    struct Vertex
    {
      Vector2f position;
      Vector2f texture_coords;
      Colorb color;
    };

    struct Face
    {
      std::array<std::uint32_t, 3> indices;
    };

    template <typename VertexType>
    struct BasicGeometry
    {
      std::uint32_t texture_id;      
      std::uint32_t level;
      std::vector<VertexType> vertices;
      std::vector<Face> faces;
    };

    using Geometry = BasicGeometry<Vertex>;
  }
}
