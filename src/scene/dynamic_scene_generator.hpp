/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <memory>

namespace ts
{
  namespace stage
  {
    class Stage;
  }

  namespace scene
  {
    class DynamicScene;

    DynamicScene generate_dynamic_scene(const stage::Stage& stage_object);
  }
}
