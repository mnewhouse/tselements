/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "cup_state.hpp"

#include "client/local_client.hpp"

namespace ts
{
  namespace client
  {
    using LocalCupState = CupState<LocalMessageDispatcher>;
  }
}
