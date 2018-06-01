/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "path_library.hpp"

namespace ts
{
  namespace resources
  {
    TrackPath* PathLibrary::create_path()
    {
      paths_.emplace_back();
      return &paths_.back();
    }

    void PathLibrary::add_style_preset(const PathStylePreset& preset)
    {
      style_presets_.push_back(preset);
    }

    const std::vector<PathStylePreset>& PathLibrary::style_presets() const noexcept
    {
      return style_presets_;
    }
  }
}