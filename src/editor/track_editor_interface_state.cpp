/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"

#include "track_editor_interface_state.hpp"

namespace ts
{
  namespace editor
  {
    namespace track
    {
      InterfaceState::InterfaceState(Tool active_tool, std::size_t mode_id)
        : active_tool_(active_tool),
          active_mode_id_(mode_id)
      {
      }

      void InterfaceState::set_active_tool(Tool tool)
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

      Tool InterfaceState::active_tool() const
      {
        return active_tool_;
      }

      std::size_t InterfaceState::active_mode() const
      {
        return active_mode_id_;
      }
    }
  }
}