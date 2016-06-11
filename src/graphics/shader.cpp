/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "shader.hpp"

#include <stdexcept>

namespace ts
{
  namespace graphics
  {
    void compile_shader(const Shader& shader, const char* data)
    {
      const char* const dummy_array[1] = { data };
      compile_shader(shader, dummy_array);
    }

    void attach_shader(const ShaderProgram& shader_program, const Shader& shader)
    {
      glAttachShader(shader_program.get(), shader.get());
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

    ShaderProgram create_shader_program(const char* vertex_shader_code, const char* fragment_shader_code)
    {
      const char* const vertex_code[] = { vertex_shader_code };
      const char* const fragment_code[] = { fragment_shader_code };
      return create_shader_program(vertex_code, fragment_code);
    }
  }
}