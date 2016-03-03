/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "shader.hpp"

#include <stdexcept>
#include <vector>

namespace ts
{
  namespace graphics
  {
    void compile_shader(const Shader& shader, const char* data)
    {
      glShaderSource(shader.get(), 1, &data, nullptr);
      glCompileShader(shader.get());

      GLint success = GL_FALSE;
      glGetShaderiv(shader.get(), GL_COMPILE_STATUS, &success);
      if (!success)
      {
        // In the case of failure, we're going to throw an exception.
        GLint max_length = 0;
        glGetShaderiv(shader.get(), GL_INFO_LOG_LENGTH, &max_length);

        std::string error(max_length, 0);
        glGetShaderInfoLog(shader.get(), max_length, &max_length, &error[0]);
        throw std::runtime_error(error);
      }
    }

    void link_shader_program(const ShaderProgram& shader_program)
    {
      glLinkProgram(shader_program.get());

      GLint success = GL_FALSE;
      glGetProgramiv(shader_program.get(), GL_LINK_STATUS, &success);
      if (!success)
      {
        GLint max_length = 0;
        glGetProgramiv(shader_program.get(), GL_INFO_LOG_LENGTH, &max_length);

        std::string error(max_length, 0);
        glGetProgramInfoLog(shader_program.get(), max_length, &max_length, &error[0]);

        throw std::runtime_error(error);
      }
    }
  }
}