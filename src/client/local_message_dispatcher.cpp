/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "local_message_dispatcher.hpp"

namespace ts
{
  namespace client
  {
    LocalMessageDispatcher::LocalMessageDispatcher(const server::MessageConveyor* message_conveyor)
      : message_conveyor_(message_conveyor)
    {
    }
  }
}