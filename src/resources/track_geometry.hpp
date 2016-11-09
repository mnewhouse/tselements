/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef VERTEX_HPP_1832593
#define VERTEX_HPP_1832593

#include "track_texture.hpp"

#include "utility/vector2.hpp"
#include "utility/color.hpp"

#include <cstdint>
#include <vector>

namespace ts
{
  namespace resources
  {
    struct TrackVertex
    {
      Vector2f position;
      Vector2f texture_coords;
      float normal;
      Colorb color;      
    };

    struct TrackFace
    {
      std::uint32_t first_index, second_index, third_index;
    };

    struct TrackGeometry
    {
      TextureId texture_id;
      std::uint32_t level;
      std::vector<TrackVertex> vertices;
    };
  }
}

#endif