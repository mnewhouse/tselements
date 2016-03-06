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
    template class CupController<server::MessageDistributor>;
  }
}