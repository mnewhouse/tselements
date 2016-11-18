/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "server_message_distributor.hpp"

#include "cup/cup_controller.hpp"

#include <utility>

namespace ts
{
  namespace server
  {
    class MessageConveyor;

    class CupController
      : public cup::CupController<DefaultMessageDistributor>
    {
    public:
      using cup::CupController<DefaultMessageDistributor>::CupController;
    };
  }
}
