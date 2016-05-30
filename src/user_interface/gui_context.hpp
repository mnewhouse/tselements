/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef GUI_CONTEXT_HPP_85918291823
#define GUI_CONTEXT_HPP_85918291823

#include "gui_renderer.hpp"

namespace ts
{
  namespace gui
  {
    class Context
    {
    public:
      const Renderer& renderer() const;

    private:
      Renderer renderer_;
    };
  }
}

#endif