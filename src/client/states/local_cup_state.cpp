/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"
#include "local_cup_state.hpp"
#include "cup_state_detail.hpp"

#include "cup/player_definition.hpp"

namespace ts
{
  namespace client
  {
    template class CupState<LocalMessageDispatcher>;
  }
}
