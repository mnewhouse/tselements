/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "viewport.hpp"

#include <vector>

#include <boost/range/iterator_range.hpp>

namespace ts
{
  namespace scene
  {
    // The ViewportArrangement class holds a number of viewports, and allows these to be
    // uniformly distributed across the screen.
    class ViewportArrangement
    {
    public:
      explicit ViewportArrangement(std::size_t max_viewports, const IntRect& screen_rect);

      std::size_t add_viewport(const world::Entity* entity);

      std::size_t viewport_count() const;
      const Viewport& viewport(std::size_t index) const;
      Viewport& viewport(std::size_t index);

      void update_viewports();

      boost::iterator_range<const Viewport*> viewports() const;

    private:
      std::vector<Viewport> viewports_;
      Viewport default_viewport_;
    };
  }
}
