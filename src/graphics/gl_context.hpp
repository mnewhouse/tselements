/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef GL_CONTEXT_HPP_665510129352
#define GL_CONTEXT_HPP_665510129352

#include <memory>

namespace ts
{
  namespace graphics
  {
    namespace gl_version
    {
      static constexpr int major = 3;
      static constexpr int minor = 2;
    }

    class RenderWindow;

    namespace detail
    {
      struct GLContextDeleter
      {
        void operator()(void* context) const;
      };
    }

    using GLContextHandle = std::unique_ptr<void, detail::GLContextDeleter>;    

    // Create a new, stand-alone context. This function assumes there's a GL context already active,
    // and will create a new context that shares its resources with the existing context.
    GLContextHandle create_gl_context();
    void activate_gl_context(const GLContextHandle& context);
  }
}

#endif