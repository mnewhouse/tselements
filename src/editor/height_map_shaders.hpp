/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef HEIGHT_MAP_SHADERS_HPP_5819281259
#define HEIGHT_MAP_SHADERS_HPP_5819281259

#include "graphics/sampler.hpp"
#include "graphics/shader.hpp"
#include "graphics/gl_check.hpp"

namespace ts
{
  namespace scene_3d
  {

    static const char* const height_map_vertex_shader_functions = R"(
uniform sampler2D u_heightMapSampler;
uniform vec2 u_heightMapCellSize;
uniform float u_heightMapMaxZ;
vec2 calculateHeightMapCoord(vec2 position)
{
  return position * u_heightMapCellSize;
}
float heightAt(vec2 position)
{
  vec2 heightMapCoord = calculateHeightMapCoord(position);
  return texture(u_heightMapSampler, heightMapCoord).a * u_heightMapMaxZ;
}
vec3 triangleNormal(vec3 a, vec3 b)
{
  vec3 result = cross(a, b);
  if (result.z < 0.0) result *= -1.0;
  return normalize(result);
}
vec3 calculateNormal(vec2 position, float z)
{
  vec2 cellSize = u_heightMapCellSize;
  float leftZ = heightAt(position - vec2(1.0, 0.0));
  float topZ = heightAt(position - vec2(0.0, 1.0));
  float rightZ = heightAt(position + vec2(1.0, 0.0));
  float bottomZ = heightAt(position + vec2(0.0, 1.0));
  vec3 normal = triangleNormal(vec3(-1.0, 0.0, z - leftZ), vec3(0.0, -1.0, z - topZ)) +
                triangleNormal(vec3(1.0, 0.0, z - rightZ), vec3(0.0, -1.0, z - topZ)) +
                triangleNormal(vec3(1.0, 0.0, z - rightZ), vec3(0.0, 1.0, z - bottomZ)) +
                triangleNormal(vec3(-1.0, 0.0, z - leftZ), vec3(0.0, 1.0, z - bottomZ));
  return normalize(normal);
})";

    struct HeightMapUniformLocations
    {
      GLuint height_map_sampler;
      GLuint height_map_cell_size;
      GLuint height_map_max_z;
    };

    inline auto get_height_map_uniform_locations(const graphics::ShaderProgram& shader_program)
    {
      HeightMapUniformLocations loc;
      loc.height_map_sampler = glCheck(glGetUniformLocation(shader_program.get(), "u_heightMapSampler"));
      loc.height_map_cell_size = glCheck(glGetUniformLocation(shader_program.get(), "u_heightMapCellSize"));
      loc.height_map_max_z = glCheck(glGetUniformLocation(shader_program.get(), "u_heightMapMaxZ"));
      return loc;
    }

    inline auto create_height_map_sampler()
    {
      GLuint sampler;
      glCheck(glCreateSamplers(1, &sampler));

      glCheck(glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
      glCheck(glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
      glCheck(glSamplerParameteri(sampler, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
      glCheck(glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
      glCheck(glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

      return graphics::Sampler(sampler);
    }
  }
}

#endif