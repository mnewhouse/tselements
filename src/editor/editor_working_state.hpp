/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <vector>
#include <cstdint>

namespace ts
{
  namespace resources
  {
    class TrackLayer;
    struct TrackPath;
  }

  namespace editor
  {
    class WorkingState
    {
    public:
      void select_layer(resources::TrackLayer* layer);
      resources::TrackLayer* selected_layer() const;

      void select_path(resources::TrackPath* path);
      resources::TrackPath* selected_path() const;

    private:
      resources::TrackLayer* selected_layer_ = nullptr;
      resources::TrackPath* selected_path_ = nullptr;
    };
  }
}