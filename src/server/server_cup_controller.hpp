/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "server/server_message_dispatcher.hpp"

#include "cup/cup_controller.hpp"

#include <utility>

namespace ts
{
  namespace server
  {
    class CupController
      : public cup::CupController<MessageDispatcher>
    {
    public:
      using cup::CupController<MessageDispatcher>::CupController;
    };
  }
}
