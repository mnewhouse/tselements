/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "server_message_dispatcher.hpp"

namespace ts
{
  namespace server
  {
    void MessageDispatcher::initiate_local_connection(const LocalConveyor* local_conveyor)
    {
      local_conveyor_ = local_conveyor;
    }
  }
}