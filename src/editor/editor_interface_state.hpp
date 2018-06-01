/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "editor_tool_types.hpp"

#include <cstdint>
#include <cstddef>

namespace ts
{
  namespace editor
  {
    enum class StateId
    {
      None,
      Editor,
      Test
    };

    class InterfaceState
    {
    public:
      explicit InterfaceState(ToolType active_tool, std::size_t mode_id = 0);

      void set_active_tool(ToolType tool);
      virtual void set_active_mode(std::size_t mode_id);

      ToolType active_tool() const;
      std::uint32_t active_mode() const;

      void set_active_state(StateId state_id);
      StateId active_state() const;

    private:
      virtual void active_tool_changed(ToolType previous, ToolType current) {}
      virtual void active_mode_changed(std::uint32_t previous, std::uint32_t current) {}

      ToolType active_tool_ = ToolType::None;
      std::uint32_t active_mode_id_ = 0;

      StateId state_id_ = StateId::None;
    };
  }
}
