/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

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
    // Make a viewport arrangement according to the entities that are controlled by the client.
    // In case of more than one controlled entity, split screen mode is used for now.
    // TODO: make a shrink-to-fit threshold.
    scene::ViewportArrangement make_viewport_arrangement(IntRect screen_rect, 
                                                         const controls::ControlCenter& control_center,
                                                         const stage::Stage& stage_obj);
  }
}
