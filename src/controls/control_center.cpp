/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "control_center.hpp"

#include <algorithm>

namespace ts
{
  namespace controls
  {
    void ControlCenter::assume_control(std::uint16_t control_slot, Controllable controllable)
    {
      control_slots_[control_slot].push_back(controllable);
    }

    void ControlCenter::release_control(std::uint16_t control_slot, Controllable controllable)
    {
      auto& slot = control_slots_[control_slot];

      // Remove all controllables with the given id.
      slot.erase(std::remove_if(slot.begin(), slot.end(),
                                [controllable](const Controllable& entry)
      {
        return entry.controllable_id() == controllable.controllable_id();
      }), slot.end());
    }

    void ControlCenter::release_control(std::uint16_t control_slot)
    {
      control_slots_[control_slot].clear();
    }

    ControlCenter::const_controllable_range ControlCenter::control_slot(std::uint16_t slot) const
    {
      auto& range = control_slots_[slot];
      return const_controllable_range(range.data(), range.data() + range.size());
    }

    ControlCenter::controllable_range ControlCenter::control_slot(std::uint16_t slot)
    {
      auto& range = control_slots_[slot];
      return controllable_range(range.data(), range.data() + range.size());
    }
  }
}