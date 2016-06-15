/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef TRACK_EDITOR_INTERFACE_STATE_HPP_668219821
#define TRACK_EDITOR_INTERFACE_STATE_HPP_668219821

#include "track_editor_tool.hpp"

#include <cstddef>

namespace ts
{
  namespace editor
  {
    namespace track
    {
      class InterfaceState
      {
      public:
        explicit InterfaceState(Tool active_tool, std::size_t mode_id = 0);

        void set_active_tool(Tool tool);
        virtual void set_active_mode(std::size_t mode_id);

        Tool active_tool() const;
        std::size_t active_mode() const;

      private:
        virtual void active_tool_changed(Tool previous, Tool current) {}
        virtual void active_mode_changed(std::size_t previous, std::size_t current) {}

        Tool active_tool_ = Tool::None;
        std::size_t active_mode_id_ = 0;
      };
    }
  }
}

#endif