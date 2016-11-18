/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <vector>
#include <cstdint>

namespace ts
{
  namespace resources
  {
    struct TrackLayer;
    struct TrackPath;
  }

  namespace editor
  {
    class WorkingState
    {
    public:
      void select_layer(resources::TrackLayer* layer);
      resources::TrackLayer* selected_layer() const;

    private:
      resources::TrackLayer* selected_layer_ = nullptr;
    };
  }
}