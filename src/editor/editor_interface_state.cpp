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
    InterfaceState::InterfaceState(ToolType active_tool, std::size_t mode_id)
      : active_tool_(active_tool),
        active_mode_id_(mode_id)
    {
    }

    void InterfaceState::set_active_tool(ToolType tool)
    {
      if (tool != active_tool_)
      {
        auto old = active_tool_;
        active_tool_ = tool;
        active_tool_changed(old, tool);
      }
    }

    void InterfaceState::set_active_mode(std::size_t mode_id)
    {
      if (mode_id != active_mode_id_)
      {
        auto old = active_mode_id_;
        active_mode_id_ = mode_id;
        active_mode_changed(old, mode_id);
      }
    }

    ToolType InterfaceState::active_tool() const
    {
      return active_tool_;
    }

    std::uint32_t InterfaceState::active_mode() const
    {
      return active_mode_id_;
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