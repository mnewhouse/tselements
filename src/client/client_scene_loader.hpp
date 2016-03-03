/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CLIENT_SCENE_LOADER_HPP_696091091234
#define CLIENT_SCENE_LOADER_HPP_696091091234

#include "scene/scene_loader.hpp"

namespace ts
{
  namespace stage
  {
    namespace messages
    {
      struct StageLoaded;
    }
  }

  namespace client
  {
    struct SceneLoaderInterface
    {
      virtual void handle_message(const stage::messages::StageLoaded& stage_loaded) = 0;
    };

    class SceneLoader
      : public SceneLoaderInterface, private scene::SceneLoader
    {
    public:
      using scene::SceneLoader::SceneLoader;
      using scene::SceneLoader::is_loading;
      using scene::SceneLoader::is_ready;
      using scene::SceneLoader::progress;
      using scene::SceneLoader::max_progress;
      using scene::SceneLoader::loading_state;
      using scene::SceneLoader::get_result;     

      virtual void handle_message(const stage::messages::StageLoaded& stage_loaded) override;
    };
  }
}

#endif