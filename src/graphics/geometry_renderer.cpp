/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "geometry_renderer.hpp"
#include "geometry.hpp"
#include "geometry_shaders.hpp"

#include <glm/gtc/type_ptr.hpp>

namespace ts
{
  namespace graphics
  {
    GeometryRenderer::GeometryRenderer()
    {
      shader_program_.reset(glCreateProgram());
      
      Shader vertex_shader(glCreateShader(GL_VERTEX_SHADER));
      Shader fragment_shader(glCreateShader(GL_FRAGMENT_SHADER));

      compile_shader(vertex_shader, geometry_vertex_shader);
      compile_shader(fragment_shader, geometry_fragment_shader);

      attach_shader(shader_program_, vertex_shader);
      attach_shader(shader_program_, fragment_shader);

      link_shader_program(shader_program_);

      GLuint sampler{};
      glCreateSamplers(1, &sampler);
      sampler_.reset(sampler);      
    }

    void GeometryRenderer::draw(const Geometry& geometry, const glm::mat4x4& view_matrix) const
    {
      glUseProgram(shader_program_.get());
      glBindSampler(0, sampler_.get());

      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      auto view_loc = glGetUniformLocation(shader_program_.get(), "u_viewMat");
      auto sampler_loc = glGetUniformLocation(shader_program_.get(), "u_textureSampler");

      glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));
      glUniform1i(sampler_loc, 0);

      geometry.draw();
      glUseProgram(0);
      glBindSampler(0, 0);
    }
  }
}