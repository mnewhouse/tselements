/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "editor_tool_types_3d.hpp"

#include <cstdint>

namespace ts
{
  namespace editor3d
  {
    struct ToolState
    {    
      ToolType active_tool = ToolType::None;
      std::uint32_t active_mode_id = 0;
    };
  }
}