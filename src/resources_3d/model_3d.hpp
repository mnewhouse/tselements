/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "utility/vector3.hpp"
#include "utility/color.hpp"

#include <cstdint>
#include <vector>

namespace ts
{
  namespace resources3d
  {
    struct Vertex
    {
      Vector3f position;
      Vector3f tex_coords;
      Vector3f normal;
      Colorb color;
    };

    struct Face
    {
      std::uint32_t a, b, c;
    };

    struct FaceVertices
    {
      const Vertex& a;
      const Vertex& b;
      const Vertex& c;
    };

    struct Model
    {
      std::vector<Vertex> vertices;
      std::vector<Face> faces;
    };

    inline FaceVertices get_face_vertices(const Face& face, const std::vector<Vertex>& vertices)
    {
      return FaceVertices
      {
        vertices[face.a],
        vertices[face.b],
        vertices[face.c]
      };
    }
  }
}