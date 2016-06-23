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

namespace ts
{
  namespace resources_3d
  {
    struct ModelVertex
    {
      Vector3f position;
      Vector3f normal;
      Vector2f tex_coords;
      Colorb color;
    };

    struct ModelFace
    {
      std::uint32_t first_index;
      std::uint32_t second_index;
      std::uint32_t third_index;
    };

    struct Model
    {
      std::vector<ModelVertex> vertices;
      std::vector<ModelFace> faces;
    };
  }
}

#endif