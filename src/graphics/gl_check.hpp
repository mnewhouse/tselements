/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <cstddef>

#ifdef TS_GL_DEBUG

#define glCheck(x) x; ts::graphics::detail::glCheckError(__FILE__, __LINE__);

#else

#define glCheck(call) call

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
