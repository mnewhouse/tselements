/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "server_stage_essentials_detail.hpp"

#include "server_message_conveyor.hpp"
#include "server_message_dispatcher.hpp"

namespace ts
{
  namespace server
  {
    template class StageEssentials<MessageDispatcher, MessageConveyor>;
  }
}