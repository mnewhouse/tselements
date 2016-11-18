/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "cup/player_definition.hpp"

#include <boost/container/small_vector.hpp>

namespace ts
{
  namespace client
  {
    struct PlayerSettings
    {
      boost::container::small_vector<cup::PlayerDefinition, 8> selected_players;
    };
  }
}
