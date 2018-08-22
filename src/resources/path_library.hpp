/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "track_path.hpp"

#include <list>
#include <vector>
#include <cstdint>

namespace ts
{
  namespace resources
  {
    struct PathStylePreset
    {
      std::string name;
      std::uint32_t id;
      PathStyle style;
    };

    class PathLibrary
    {
    public:
      TrackPath* create_path();
      TrackPath* create_path(std::uint32_t id);
      const std::list<TrackPath>& paths() const noexcept;

      void add_style_preset(const PathStylePreset& preset);
      const std::vector<PathStylePreset>& style_presets() const noexcept;

      TrackPath* find_path(std::uint32_t id);
      const TrackPath* find_path(std::uint32_t id) const;

    private:
      // We use a list to ensure reference stability. We don't need random access here.
      std::list<TrackPath> paths_;
      std::vector<PathStylePreset> style_presets_;
      std::uint32_t max_path_id_ = 0;
    };
  }
}