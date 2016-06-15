/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef SCENE_3D_SHADERS_HPP_5819281259
#define SCENE_3D_SHADERS_HPP_5819281259

namespace ts
{
  namespace scene_3d
  {
    static const char* const version_string = "#version 330\n";

    static const char* const terrain_vertex_shader = R"(
uniform mat4 u_projectionMatrix;
uniform mat4 u_viewMatrix;
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_texCoord;
layout(location = 2) in vec3 in_normal;
out vec3 frag_texCoord;
out vec3 frag_normal;
out vec4 frag_color;
void main()
{
  float z = heightAt(in_position.xy) + in_position.z;
  frag_texCoord = in_texCoord;  
  frag_color = vec4(1.0, 1.0, 1.0, 1.0);
  frag_normal = (calculateNormal(in_position.xy, z) + in_normal) * 0.5;
  gl_Position = u_projectionMatrix * u_viewMatrix * vec4(in_position.xy, z, 1.0);
}
)";

    static const char* const terrain_fragment_shader = R"(
uniform sampler2DArray u_textureSampler;
in vec3 frag_texCoord;
in vec3 frag_normal;
in vec4 frag_color;
out vec4 out_fragColor;
void main()
{
  float cosTheta = clamp(dot(frag_normal, normalize(vec3(-1, -1, 3.0))), 0.0, 1.0);
  vec4 tex_color = texture(u_textureSampler, frag_texCoord);
  float alpha = tex_color.a;
  out_fragColor = vec4(tex_color.rgb * cosTheta * cosTheta, alpha) * frag_color;
}
)";
  }
}

#endif