/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef GL_SCISSOR_HPP_81298129
#define GL_SCISSOR_HPP_81298129

#include "utility/vector2.hpp"
#include "utility/rect.hpp"

#include "graphics/gl_check.hpp"

#include <GL/glew.h>
#include <GL/GL.h>

namespace ts
{
  namespace graphics
  {
    inline void scissor_box(Vector2u screen_size, IntRect view_port)
    {
      glCheck(glEnable(GL_SCISSOR_TEST));
      glCheck(glScissor(view_port.left, screen_size.y - view_port.height - view_port.top, view_port.width,
                        view_port.height));
    }

    inline void disable_scissor_box()
    {
      glCheck(glDisable(GL_SCISSOR_TEST));
    }
  }
}

#endif