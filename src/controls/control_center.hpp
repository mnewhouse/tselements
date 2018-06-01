/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "controllable.hpp"

#include <cstdint>
#include <array>

#include <boost/container/small_vector.hpp>
#include <boost/range/iterator_range.hpp>

namespace ts
{
  namespace controls
  {
    static constexpr std::uint32_t max_control_slots = 4;

    // The ControlCenter class template can map controllables to a fixed number of control slots.
    class ControlCenter
    {
    public:
      void assume_control(std::uint16_t control_slot, Controllable controllable);
      void release_control(std::uint16_t control_slot, Controllable controllable);
      void release_control(std::uint16_t control_slot);

      using const_controllable_range = boost::iterator_range<const Controllable*>;
      using controllable_range = boost::iterator_range<Controllable*>;

      // This function gets all controllable ids for a given slot id,
      // in the form of an iterator range.
      const_controllable_range control_slot(std::uint16_t control_slot) const;
      controllable_range control_slot(std::uint16_t control_slot);

    public:
      std::array<boost::container::small_vector<Controllable, 16>, max_control_slots> control_slots_;
    };
  }
}
