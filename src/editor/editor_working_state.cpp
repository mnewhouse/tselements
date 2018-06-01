/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "editor_working_state.hpp"

namespace ts
{
  namespace editor
  {
    resources::TrackLayer* WorkingState::selected_layer() const
    {
      return selected_layer_;
    }

    void WorkingState::select_layer(resources::TrackLayer* layer)
    {
      selected_layer_ = layer;
    }

    resources::TrackPath* WorkingState::selected_path() const
    {
      return selected_path_;
    }

    void WorkingState::select_path(resources::TrackPath* path)
    {
      selected_path_ = path;
    }
  }
}