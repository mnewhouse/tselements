/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "terrain_map.hpp"

#include "resources/pattern_store.hpp"

#include <algorithm>
#include <vector>
#include <iterator>
#include <utility>

namespace ts
{
  namespace world
  {
    template <typename MapComponentRange>
    TerrainMap::TerrainMap(const MapComponentRange& components, resources::PatternStore pattern_store)
      : pattern_store_(std::move(pattern_store))
    {
      using std::begin;
      using std::end;

      std::vector<TerrainMapComponent> component_buffer(begin(components), end(components));
      std::sort(component_buffer.begin(), component_buffer.end(),
                [](const auto& a, const auto& b)
      {
        return std::tie(a.level, a.z_index) < std::tie(b.level, b.z_index);
      });

      std::vector<value_type> tree_value_buffer;
      for (auto it = component_buffer.begin(); it != component_buffer.end(); )
      {
        auto range_end = std::upper_bound(std::next(it), component_buffer.end(), it->level,
                                          [](auto level, const auto& elem)
        {
          return level < elem.level;
        });

        tree_value_buffer.clear();
        std::transform(it, range_end, std::back_inserter(tree_value_buffer),
                       [](const auto& elem)
        {
          return std::make_pair(bounding_box(elem), elem);
        });

        component_maps_.resize(it->level); // Make sure "empty" levels get their own entry
        component_maps_.emplace_back(tree_value_buffer.begin(), tree_value_buffer.end());
        it = range_end;
      }

      terrain_buffer_.reserve(component_buffer.size());
    }
  }
}
