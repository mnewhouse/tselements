/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"
#include "controllable.hpp"

namespace ts
{
  namespace controls
  {
    Controllable::Controllable(std::uint16_t controllable_id)
      : controllable_id_(controllable_id)
    {
    }

    bool Controllable::set_control_state(Control control, bool state)
    {
      bool prior_state = control_state(control);
      if (prior_state == state) return false;
      
      // If the earlier state is not the same as the new state, we can simply use bitwise XOR.
      controls_mask_ ^= control;
      return true;
    }

    bool Controllable::control_state(Control control) const
    {
      return (controls_mask_ & control) != 0;
    }

    std::uint16_t Controllable::controllable_id() const
    {
      return controllable_id_;
    }

    std::uint16_t Controllable::controls_mask() const
    {
      return controls_mask_;
    }

    void Controllable::update_controls_mask(std::uint16_t controls_mask)
    {
      controls_mask_ = controls_mask;
    }
  }
}