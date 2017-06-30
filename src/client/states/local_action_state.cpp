/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "action_state.hpp"
#include "action_state_detail.hpp"

#include "client/local_message_dispatcher.hpp"

namespace ts
{
  namespace client
  {
    template class ActionState<LocalMessageDispatcher>;
  }
}