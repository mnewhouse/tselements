/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/


#include "viewport_arrangement.hpp"

namespace ts
{
  namespace scene
  {
    ViewportArrangement::ViewportArrangement(std::size_t max_viewports, const IntRect& screen_rect)
      : default_viewport_(screen_rect)
    {
      viewports_.reserve(max_viewports);
    }

    Viewport& ViewportArrangement::viewport(std::size_t index)
    {
      if (viewports_.empty())
      {
        return default_viewport_;
      }

      return viewports_[index];
    }

    const Viewport& ViewportArrangement::viewport(std::size_t index) const
    {
      if (viewports_.empty())
      {
        return default_viewport_;
      }

      return viewports_[index];
    }

    std::size_t ViewportArrangement::add_viewport(const world::Entity* entity)
    {
      if (viewports_.size() == viewports_.capacity())
      {
        throw std::runtime_error("too many viewports requested");
      }

      auto index = viewports_.size();
      viewports_.push_back(default_viewport_);
      viewports_.back().camera().follow_entity(entity);
	  
      return index;
    }

    std::size_t ViewportArrangement::viewport_count() const
    {
      return viewports_.size();
    }

    void ViewportArrangement::update_viewports()
    {
      // If we don't have any viewports, update the default one.
      if (viewports_.empty())
      {
        default_viewport_.update_camera();
      }

      else
      {
        for (auto& viewport : viewports_)
        {
          viewport.update_camera();
        }
      }
    }

    boost::iterator_range<const Viewport*> ViewportArrangement::viewports() const
    {
      if (viewports_.empty())
      {
        return boost::make_iterator_range(&default_viewport_, &default_viewport_ + 1);
      }

      return boost::make_iterator_range(viewports_.data(), viewports_.data() + viewports_.size());
    }
  }
}