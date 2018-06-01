/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "utility/vector2.hpp"
#include "utility/rect.hpp"

#include "graphics/gl_check.hpp"

#include <GL/glew.h>
#include <GL/GL.h>

namespace ts
{
  namespace graphics
  {
    inline void scissor_box(IntRect view_port, Vector2i screen_size)
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
