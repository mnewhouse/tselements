/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CONTROLLABLE_HPP_7110923
#define CONTROLLABLE_HPP_7110923

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
      bool control_state(Control control) const;

      void update_controls_mask(std::uint16_t controls_mask);

      std::uint16_t controls_mask() const;
      std::uint16_t controllable_id() const;

    private:
      std::uint16_t controls_mask_ = 0;
      std::uint16_t controllable_id_;
    };
  }
}

#endif