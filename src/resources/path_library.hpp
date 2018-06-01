/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "track_path.hpp"

#include <list>
#include <vector>

namespace ts
{
  namespace resources
  {
    struct PathStylePreset
    {
      std::string name;
      PathStyle style;
    };

    class PathLibrary
    {
    public:
      TrackPath* create_path();

      void add_style_preset(const PathStylePreset& preset);
      const std::vector<PathStylePreset>& style_presets() const noexcept;

    private:
      // We use a list to ensure reference stability. We don't need random access here.
      std::list<TrackPath> paths_;
      std::vector<PathStylePreset> style_presets_;
    };
  }
}