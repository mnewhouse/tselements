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
    void MessageDispatcher::initiate_local_connection(const client::MessageConveyor* client_message_conveyor)
    {
      local_client_ = client_message_conveyor;
    }
  }
}