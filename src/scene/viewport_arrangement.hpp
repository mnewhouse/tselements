/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef VIEWPORT_ARRANGEMENT_HPP_689123819
#define VIEWPORT_ARRANGEMENT_HPP_689123819

#include "viewport.hpp"

#include <vector>

namespace ts
{
  namespace scene
  {
    // The ViewportArrangement class holds a number of viewports, and allows these to be
    // uniformly distributed across the screen.
    class ViewportArrangement
    {
    public:
      explicit ViewportArrangement(std::size_t max_viewports, const DoubleRect& screen_rect);

      std::size_t add_viewport(const world::Entity* entity);

      std::size_t viewport_count() const;
      const Viewport& viewport(std::size_t index) const;
      Viewport& viewport(std::size_t index);

      void update_viewports();

    private:
      std::vector<Viewport> viewports_;
      Viewport default_viewport_;
    };
  }
}

#endif