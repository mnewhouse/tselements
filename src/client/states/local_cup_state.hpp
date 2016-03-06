/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef LOCAL_CUP_STATE_HPP_2189124929
#define LOCAL_CUP_STATE_HPP_2189124929

#include "cup_state.hpp"

#include "client/local_client.hpp"

namespace ts
{
  namespace client
  {
    using LocalCupState = CupState<LocalMessageDispatcher>;
  }
}

#endif