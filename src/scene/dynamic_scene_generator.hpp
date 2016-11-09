/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef DYNAMIC_SCENE_GENERATOR_HPP_1910294
#define DYNAMIC_SCENE_GENERATOR_HPP_1910294

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


#endif