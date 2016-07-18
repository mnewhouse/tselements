/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef MODEL_3D_HPP_849174827174
#define MODEL_3D_HPP_849174827174

#include "utility/color.hpp"
#include "utility/vector2.hpp"
#include "utility/vector3.hpp"

#include <vector>
#include <array>
#include <cstdint>
#include <type_traits>

namespace ts
{
  namespace resources_3d
  {
    struct ModelVertex
    {
      Vector3f position;
      Vector3f tex_coords;
      Vector3<std::int8_t> normal;
      Colorb color;
    };

    struct ModelFace
    {
      std::uint32_t first_index;
      std::uint32_t second_index;
      std::uint32_t third_index;
    };

    template <typename VertexType>
    struct BasicFaceVertices
    {
      const VertexType& first;
      const VertexType& second;
      const VertexType& third;
    };

    using FaceVertices = BasicFaceVertices<ModelVertex>;

    struct ModelLine
    {
      std::uint32_t first_index;
      std::uint32_t second_index;
    };

    template <typename VertexType>
    struct BasicModel
    {
      std::vector<VertexType> vertices;
      std::vector<ModelFace> faces;
      std::vector<ModelLine> lines;
    };

    using Model = BasicModel<ModelVertex>;

    template <typename VertexContainer>
    inline auto face_vertices(const VertexContainer& vertices, ModelFace face)
      -> BasicFaceVertices<std::decay_t<decltype(vertices[0])>>
    {
      return
      
      {
        vertices[face.first_index],
        vertices[face.second_index],
        vertices[face.third_index]
      };
    }

    template <typename VertexType>
    inline auto face_vertices(const BasicModel<VertexType>& model, ModelFace face)
    {
      return face_vertices(model.vertices, face);
    }
  }
}

#endif