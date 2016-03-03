/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CUP_STATE_INTERFACE_HPP_48494819812
#define CUP_STATE_INTERFACE_HPP_48494819812

#include "cup/cup_message_fwd.hpp"

namespace ts
{
  namespace client
  {
    struct CupStateInterface
    {
      virtual void handle_message(const cup::messages::StageBegin& stage_begin) {}
    };
  }
}

#endif