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
      paths_.back().id = ++max_path_id_;
      return &paths_.back();
    }

    TrackPath* PathLibrary::create_path(std::uint32_t id)
    {
      paths_.emplace_back();
      paths_.back().id = id;
      return &paths_.back();

      if (id > max_path_id_)
      {
        max_path_id_ = id;
      }          
    }

    const TrackPath* PathLibrary::find_path(std::uint32_t id) const
    {
      auto it = std::find_if(paths_.begin(), paths_.end(), [=](const TrackPath& p)
      {
        return p.id == id;
      });

      if (it == paths_.end())
      {
        return nullptr;
      }

      return &*it;
    }

    TrackPath* PathLibrary::find_path(std::uint32_t id)
    {
      auto it = std::find_if(paths_.begin(), paths_.end(), [=](const TrackPath& p)
      {
        return p.id == id;
      });

      if (it == paths_.end())
      {
        return nullptr;
      }

      return &*it;
    }

    void PathLibrary::add_style_preset(const PathStylePreset& preset)
    {
      style_presets_.push_back(preset);
      style_presets_.back().id = style_presets_.size();
    }

    const std::vector<PathStylePreset>& PathLibrary::style_presets() const noexcept
    {
      return style_presets_;
    }
    
    const std::list<TrackPath>& PathLibrary::paths() const noexcept
    {
      return paths_;
    }
  }
}