/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "graphics/shader.hpp"

namespace ts
{
  namespace scene3d
  {
    namespace shaders
    {
      constexpr const char terrain_vertex_shader[] =
        R"(
#version 330
uniform mat4 u_viewMatrix;
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_texCoords;
layout(location = 2) in vec3 in_normal;
layout(location = 3) in vec4 in_color;
out vec3 frag_texCoords;
out vec3 frag_normal;
out vec4 frag_color;
void main()
{
  gl_Position = u_viewMatrix * vec4(in_position, 1);
  frag_texCoords = in_texCoords;
  frag_normal = in_normal;
  frag_color = in_color;
}
)";

      constexpr const char terrain_fragment_shader[] =
        R"(
#version 330
uniform sampler2DArray u_textureSampler;
uniform vec3 u_lightDirection;
in vec3 frag_texCoords;
in vec3 frag_normal;
in vec4 frag_color;
out vec4 out_color;
void main()
{
  float lightness = clamp(dot(u_lightDirection, frag_normal), 0, 1);
  vec4 tex_color = texture(u_textureSampler, frag_texCoords);
  out_color = vec4(frag_color.rgb * lightness, frag_color.a) * tex_color;
}
)";

      struct TerrainShaderUniforms
      {
        std::uint32_t view_matrix;
        std::uint32_t texture_sampler;
        std::uint32_t light_direction;        
      };

      inline auto terrain_shader_uniform_locations(const graphics::ShaderProgram& prog)
      {
        TerrainShaderUniforms result;

        auto p = prog.get();
        result.view_matrix = glGetUniformLocation(p, "u_viewMatrix");
        result.texture_sampler = glGetUniformLocation(p, "u_textureSampler");
        result.light_direction = glGetUniformLocation(p, "u_lightDirection");
        return result;
      }
    }
  }
}