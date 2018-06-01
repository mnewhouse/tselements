/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/


#include "server_cup_controller.hpp"
#include "server_message_dispatcher.hpp"

#include "cup/cup_controller_detail.hpp"

namespace ts
{
  namespace cup
  {
    template class CupController<server::MessageDispatcher>;
  }
}