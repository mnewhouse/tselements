/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"

#include "gl_check.hpp"

#include <GL/glew.h>

#include <iostream>

namespace ts
{
  namespace graphics
  {
    namespace detail
    {
      void glCheckError(const char* file, std::size_t line)
      {
        auto error_code = glGetError();
        if (error_code != GL_NO_ERROR)
        {
          std::cerr << "OpenGL error in '" << file << "' on line " << line << " (code: " <<
            error_code << std::endl;
        }        
      }
    }
  }
}