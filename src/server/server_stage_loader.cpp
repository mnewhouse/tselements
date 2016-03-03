/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "server_stage_loader.hpp"
#include "server_message_conveyor.hpp"

#include "cup/cup_messages.hpp"

#include "stage/stage.hpp"

namespace ts
{
  namespace server
  {
    void StageLoader::handle_message(cup::messages::PreInitialization&& pre_initialization)
    {
      async_load(std::move(pre_initialization.stage_description));
    }
  }
}