/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CLIENT_VIEWPORT_ARRANGEMENT_HPP_5891829812
#define CLIENT_VIEWPORT_ARRANGEMENT_HPP_5891829812

#include "scene/viewport_arrangement.hpp"

#include "utility/rect.hpp"

namespace ts
{
  namespace controls
  {
    class ControlCenter;
  }

  namespace stage
  {
    class Stage;
  }

  namespace client
  {
    class ViewportArrangement
      : public scene::ViewportArrangement
    {
    public:
      explicit ViewportArrangement(std::size_t max_viewports, DoubleRect screen_rect,
                                   const controls::ControlCenter& control_center, const stage::Stage& stage_obj);

    private:
    };
  }
}

#endif