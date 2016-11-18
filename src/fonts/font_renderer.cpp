/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"

#include "font_renderer.hpp"
#include "text_shaders.hpp"
#include "text_geometry.hpp"

#include <glm/gtc/type_ptr.hpp>

namespace ts
{
  namespace fonts
  {
    FontRenderer::FontRenderer()
    {
      shader_program_.reset(glCreateProgram());

      graphics::Shader vertex_shader(glCreateShader(GL_VERTEX_SHADER));
      graphics::Shader fragment_shader(glCreateShader(GL_FRAGMENT_SHADER));

      graphics::compile_shader(vertex_shader, text_vertex_shader);
      graphics::compile_shader(fragment_shader, text_fragment_shader);

      graphics::attach_shader(shader_program_, vertex_shader);
      graphics::attach_shader(shader_program_, fragment_shader);

      graphics::link_shader_program(shader_program_);

      GLuint sampler;
      glCreateSamplers(1, &sampler);
      sampler_.reset(sampler);

      glSamplerParameterf(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glSamplerParameterf(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    void FontRenderer::draw(const TextGeometry& text_geometry, const glm::mat4& view_matrix) const
    {
      glUseProgram(shader_program_.get());
      glBindSampler(0, sampler_.get());

      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      auto view_loc = glGetUniformLocation(shader_program_.get(), "u_viewMat");
      auto sampler_loc = glGetUniformLocation(shader_program_.get(), "u_textureSampler");

      glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));
      glUniform1i(sampler_loc, 0);

      text_geometry.draw();
      glUseProgram(0);
      glBindSampler(0, 0);
    }
  }
}