/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef STAGE_MESSAGES_HPP_125891258
#define STAGE_MESSAGES_HPP_125891258

#include <cstdint>

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
        std::uint16_t controls_mask;
      };
    }
  }
}

#endif