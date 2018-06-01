/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "control.hpp"

#include <boost/container/small_vector.hpp>
#include <boost/range/iterator_range.hpp>

#include <map>

namespace ts
{
  namespace controls
  {
    // The KeyMapping class maps a key code to zero or more control/slot combinations.
    // Keys must be lookuppable (is that a word? I think not) in an efficient manner.
    template <typename KeyCode>
    class KeyMapping
    {
    public:
      struct Entry
      {
        Control control;
        std::uint32_t slot;
      };

      using entry_range = boost::iterator_range<const Entry*>;
      entry_range controls_by_key(KeyCode key_code) const;

      void define_control(KeyCode key_code, Control control, std::uint32_t slot);
      
    private:
      std::map<KeyCode, boost::container::small_vector<Entry, 16>> key_mapping_;
    };

    template <typename KeyCode>
    typename KeyMapping<KeyCode>::entry_range KeyMapping<KeyCode>::controls_by_key(KeyCode key_code) const
    {
      auto it = key_mapping_.find(key_code);
      if (it == key_mapping_.end()) return entry_range(nullptr, nullptr);

      return entry_range(it->second.data(), it->second.data() + it->second.size());
    }

    template <typename KeyCode>
    void KeyMapping<KeyCode>::define_control(KeyCode key_code, Control control, std::uint32_t slot)
    {
      key_mapping_[key_code].push_back({ control, slot });
    }
  }
}
