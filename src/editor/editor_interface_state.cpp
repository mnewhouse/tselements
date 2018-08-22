/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/


#include "editor_interface_state.hpp"

#include <cstddef>

namespace ts
{
  namespace editor
  {
    InterfaceState::InterfaceState(ModeType active_mode, std::size_t tool_id)
      : active_mode_(active_mode),
        active_tool_id_(tool_id)
    {
    }

    void InterfaceState::set_active_mode(ModeType mode)
    {
      if (mode != active_mode_)
      {
        auto old = active_mode_;
        active_mode_ = mode;
        active_mode_changed(old, mode);
      }
    }

    void InterfaceState::set_active_tool(std::size_t tool_id)
    {
      if (tool_id != active_tool_id_)
      {
        auto old = active_tool_id_;
        active_tool_id_ = tool_id;
        active_tool_changed(old, tool_id);
      }
    }

    ModeType InterfaceState::active_mode() const
    {
      return active_mode_;
    }

    std::uint32_t InterfaceState::active_tool() const
    {
      return active_tool_id_;
    }

    void InterfaceState::set_active_state(StateId state_id)
    {
      state_id_ = state_id;
    }

    StateId InterfaceState::active_state() const
    {
      return state_id_;
    }
  }
}