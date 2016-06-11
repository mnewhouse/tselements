/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef GL_CHECK_HPP_384599183245
#define GL_CHECK_HPP_384599183245

#include <cstddef>

#ifdef TS_GL_DEBUG

#define glCheck(x) x; ts::graphics::detail::glCheckError(__FILE__, __LINE__);

#else

#define glCheck(call) (call)

#endif

namespace ts
{
  namespace graphics
  {
    namespace detail
    {
      void glCheckError(const char* file, std::size_t line);
    }
  }
}

#endif