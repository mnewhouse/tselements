/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

namespace ts
{
  namespace resources3d
  {
    struct PathLayer;
  }

  namespace editor3d
  {
    struct WorkingState
    {
      resources3d::PathLayer* selected_path_layer = nullptr;
    };
  }
}
