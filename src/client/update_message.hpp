/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef UPDATE_MESSAGE_HPP_48192818923
#define UPDATE_MESSAGE_HPP_48192818923

#include <cstdint>

namespace ts
{
  namespace client
  {
    namespace messages
    {
      // Message type that's used internally to request an update in other logical components.
      struct Update
      {
        std::uint32_t frame_duration;
      };
    }
  }
}

#endif