/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "control.hpp"

namespace ts
{
  namespace controls
  {
    // The Controllable base class keeps a bitmask to store information
    // about which controls are pressed, plus a simple interface
    // to examine and alter the state.
    class Controllable
    {
    public:
      explicit Controllable(std::uint16_t controllable_id);

      bool set_control_state(Control control, bool state);
      bool set_control_state(FreeControl control, std::uint8_t value);
      bool set_control_state(FreeControl control, float value);

      std::uint8_t control_state(FreeControl control) const;
      std::uint8_t control_state(Control control) const;

      void update_controls_mask(ControlsMask controls_mask);

      ControlsMask controls_mask() const;
      std::uint16_t controllable_id() const;

    private:
      ControlsMask controls_mask_ = {};
      std::uint16_t controllable_id_;
    };
  }
}
