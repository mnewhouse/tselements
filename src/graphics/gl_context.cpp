/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "gl_context.hpp"
#include "render_window.hpp"

#include <GL/glew.h>

#ifdef WIN32
#include "GL/wglew.h"
#endif

#include <SFML/Window/Context.hpp>

#include <exception>
#include <stdexcept>


namespace ts
{
  namespace graphics
  {
    namespace detail
    {
      void GLContextDeleter::operator()(void* context) const
      {
        std::default_delete<sf::Context> deleter;
        deleter(static_cast<sf::Context*>(context));
      }
    }

    static std::runtime_error context_creation_error(const char* specific_error)
    {
      std::string error = "failed to create GL context: ";
      error += specific_error;
      throw std::runtime_error(error);
    }

    GLContextHandle create_gl_context()
    {
      sf::ContextSettings settings;
      settings.majorVersion = gl_version::major;
      settings.minorVersion = gl_version::minor;

      auto context = new sf::Context(settings, 0, 0);      
      return GLContextHandle(context);
    }

    void activate_gl_context(const GLContextHandle& context)
    {
      static_cast<sf::Context*>(context.get())->setActive(true);
    }
  }
}