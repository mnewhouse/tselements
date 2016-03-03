/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "server_cup_controller.hpp"

#include "cup/cup_controller_detail.hpp"

namespace ts
{
  namespace cup
  {
    template class CupController<server::CupControllerMessageDispatcher>;
  }

  namespace server
  {
    CupControllerMessageDispatcher::CupControllerMessageDispatcher(const MessageDispatcher* message_dispatcher,
                                                                   const MessageConveyor* message_conveyor)
      : dispatcher_(message_dispatcher),
        message_conveyor_(message_conveyor)
    {}
  }
}