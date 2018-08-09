/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "controllable.hpp"

#include "utility/math_utilities.hpp"

namespace ts
{
  namespace controls
  {
    template <typename Mask, typename ControlType>
    static auto free_control_var(Mask& mask, ControlType control) -> decltype(&mask.throttle)
    {
      switch (control)
      {
      case ControlType::Left: return &mask.left;
      case ControlType::Right: return &mask.right;
      case ControlType::Throttle: return &mask.throttle;
      case ControlType::Brake: return &mask.brake;
      default: return nullptr;
      }
    }

    Controllable::Controllable(std::uint16_t controllable_id)
      : controllable_id_(controllable_id)
    {
    }

    bool Controllable::set_control_state(Control control, bool state)
    {      
      if (auto var = free_control_var(controls_mask_, control))
      {
        auto old_state = *var;
        *var = state ? 255 : 0;
        return *var != old_state;
      }

      const auto c = static_cast<std::uint16_t>(control);
      if (((controls_mask_.other & c) == c) != state)
      {      
        if (state) controls_mask_.other |= static_cast<std::uint16_t>(control);
        else controls_mask_.other &= ~static_cast<std::uint16_t>(control);
        return true;
      }

      return false;
    }

    bool Controllable::set_control_state(FreeControl control, std::uint8_t value)
    {
      if (auto var = free_control_var(controls_mask_, control))
      {
        auto old_state = *var;
        *var = value;
        return value != old_state;
      }

      return false;
    }

    bool Controllable::set_control_state(FreeControl control, float value)
    {
      auto v = static_cast<std::uint8_t>(clamp(value, 0.0f, 1.0f) * 255.0f);

      return set_control_state(control, v);
    }

    std::uint8_t Controllable::control_state(Control control) const
    {
      if (auto var = free_control_var(controls_mask_, control))
      {
        return *var;
      }

      const auto c = static_cast<std::uint16_t>(control);
      const bool state = (controls_mask_.other & c) == c;
      return state ? 1 : 0;
    }

    std::uint8_t Controllable::control_state(FreeControl control) const
    {
      if (auto var = free_control_var(controls_mask_, control))
      {
        return *var;
      }

      return 0;
    }

    std::uint16_t Controllable::controllable_id() const
    {
      return controllable_id_;
    }

    ControlsMask Controllable::controls_mask() const
    {
      return controls_mask_;
    }

    void Controllable::update_controls_mask(ControlsMask controls_mask)
    {
      controls_mask_ = controls_mask;
    }
  }
}