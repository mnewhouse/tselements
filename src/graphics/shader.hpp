/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef SHADER_HPP_34189239
#define SHADER_HPP_34189239

#include <GL/glew.h>

#include <memory>
#include <string>
#include <stdexcept>

namespace ts
{
  namespace graphics
  {
    namespace detail
    {
      // Various GL deleters
      struct ShaderDeleter
      {
        using pointer = GLuint;
        void operator()(pointer shader) const
        {
          glDeleteShader(shader);
        }
      };

      struct ShaderProgramDeleter
      {
        using pointer = GLuint;
        void operator()(pointer program) const
        {
          glDeleteProgram(program);
        }
      };
    }

    using Shader = std::unique_ptr<GLuint, detail::ShaderDeleter>;
    using ShaderProgram = std::unique_ptr<GLuint, detail::ShaderProgramDeleter>;

    template <std::size_t N>
    void compile_shader(const Shader& shader, const char* const (&source)[N])
    {
      glShaderSource(shader.get(), N, source, nullptr);
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

    template <std::size_t N1, std::size_t N2>
    ShaderProgram create_shader_program(const char* const (&fragment_shader_code)[N1],
                                        const char* const (&vertex_shader_code)[N2])
    {
      ShaderProgram shader_program(glCreateProgram());

      Shader vertex_shader(glCreateShader(GL_VERTEX_SHADER));
      Shader fragment_shader(glCreateShader(GL_FRAGMENT_SHADER));

      compile_shader(vertex_shader, fragment_shader_code);
      compile_shader(fragment_shader, vertex_shader_code);

      attach_shader(shader_program, vertex_shader);
      attach_shader(shader_program, fragment_shader);

      link_shader_program(shader_program);

      return shader_program;
    }

    // Utility function to compile a shader from source, and throw an exception on failure.
    void compile_shader(const Shader& shader, const char* source);
    void attach_shader(const ShaderProgram& shader_program, const Shader& shader);
    void link_shader_program(const ShaderProgram& shader_program);

    ShaderProgram create_shader_program(const char* vertex_shader_code, const char* fragment_shader_code);
  }
}

#endif