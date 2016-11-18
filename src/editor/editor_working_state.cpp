/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"
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
  }
}