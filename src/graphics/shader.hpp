/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef SHADER_HPP_34189239
#define SHADER_HPP_34189239

#include <GL/glew.h>

#include <memory>

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

      struct SamplerDeleter
      {
        using pointer = GLuint;
        void operator()(pointer sampler) const
        {
          glDeleteSamplers(1, &sampler);
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
    using Sampler = std::unique_ptr<GLuint, detail::SamplerDeleter>;

    // Utility function to compile a shader from source, and throw an exception on failure.
    void compile_shader(const Shader& shader, const char* source);
    void link_shader_program(const ShaderProgram& shader_program);
  }
}

#endif