/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef TERRAIN_INTERFACE_SHADERS_HPP_48934819734
#define TERRAIN_INTERFACE_SHADERS_HPP_48934819734

namespace ts
{
  namespace editor
  {
    static const char* const terrain_interface_vertex_shader = R"(
uniform mat4 u_projectionMatrix;
uniform mat4 u_viewMatrix;
layout(location = 0) in vec2 in_position;
layout(location = 1) in vec4 in_color;
out vec4 frag_color;
void main()
{
    float z = heightAt(in_position);
    gl_Position = u_projectionMatrix * u_viewMatrix * vec4(in_position, z, 1.0);
    frag_color = in_color;
}
)";

    static const char* const terrain_interface_fragment_shader = R"(
in vec4 frag_color;
out vec4 out_fragColor;
void main()
{
  out_fragColor = frag_color;
}
)";
  }
}

#endif