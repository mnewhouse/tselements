/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

namespace ts
{
  namespace graphics
  {
    static const char* const geometry_vertex_shader = R"(
#version 330
uniform mat4 u_viewMat;
layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_texCoord;
layout(location = 2) in vec4 in_color;
out vec2 frag_texCoord;
out vec4 frag_color;
void main()
{
  gl_Position = u_viewMat * vec4(in_position.xy, 0.0, 1.0);
  frag_texCoord = in_texCoord;
  frag_color = in_color; 
}
)";

    static const char* const geometry_fragment_shader = R"(
#version 330
uniform sampler2D u_textureSampler;
in vec2 frag_texCoord;
in vec4 frag_color;
out vec4 out_fragColor;
void main()
{
  out_fragColor = texture(u_textureSampler, frag_texCoord) * frag_color;
}
)";
  }
}