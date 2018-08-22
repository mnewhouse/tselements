/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "editor_mode_types.hpp"

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
      explicit InterfaceState(ModeType active_mode, std::size_t tool_id = 0);

      void set_active_mode(ModeType mode);
      virtual void set_active_tool(std::size_t mode_id);

      ModeType active_mode() const;
      std::uint32_t active_tool() const;

      void set_active_state(StateId state_id);
      StateId active_state() const;

    private:
      virtual void active_mode_changed(ModeType previous, ModeType current) {}
      virtual void active_tool_changed(std::uint32_t previous, std::uint32_t current) {}

      ModeType active_mode_ = ModeType::None;
      std::uint32_t active_tool_id_ = 0;

      StateId state_id_ = StateId::None;
    };
  }
}
