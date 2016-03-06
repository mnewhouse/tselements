/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef SERVER_CUP_CONTROLLER_HPP_2289128359
#define SERVER_CUP_CONTROLLER_HPP_2289128359

#include "server_message_distributor.hpp"

#include "cup/cup_controller.hpp"

#include <utility>

namespace ts
{
  namespace server
  {
    class MessageConveyor;

    class CupController
      : public cup::CupController<MessageDistributor>
    {
    public:
      using cup::CupController<MessageDistributor>::CupController;
    };
  }
}

#endif