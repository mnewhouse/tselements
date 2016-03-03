/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CLIENT_SCENE_LOADER_HPP_3839898125
#define CLIENT_SCENE_LOADER_HPP_3839898125

#include "client_scene_loader.hpp"

#include "stage/stage_messages.hpp"

namespace ts
{
  namespace client
  {
    void SceneLoader::handle_message(const stage::messages::StageLoaded& stage_loaded)
    {
      async_load_scene(stage_loaded.stage_ptr);
    }
  }
}

#endif