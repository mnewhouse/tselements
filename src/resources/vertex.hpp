/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef VERTEX_HPP_1832593
#define VERTEX_HPP_1832593

#include <utility/vertex.hpp>

#include "texture.hpp"

#include <cstdint>

namespace ts
{
  namespace resources
  {
    using Vertex = ts::Vertex<float, std::uint8_t>;

    struct VertexArray
    {
      TextureId texture_id;
      std::vector<Vertex> vertices;
    };
  }
}

#endif