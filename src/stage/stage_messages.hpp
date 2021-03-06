/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <cstdint>

#include "controls/control.hpp"

namespace ts
{
  namespace stage
  {
    class Stage;

    namespace messages
    {
      struct StageLoaded
      {
        const Stage* stage_ptr;
      };

      struct ControlUpdate
      {
        std::uint32_t stage_time;
        std::uint16_t controllable_id;
        controls::ControlsMask controls_mask;
      };
    }
  }
}
