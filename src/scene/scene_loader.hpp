/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef SCENE_LOADER_HPP_2198129815
#define SCENE_LOADER_HPP_2198129815

#include "utility/generic_loader.hpp"

#include <boost/optional.hpp>

namespace ts
{
  namespace stage
  {
    class Stage;
  }

  namespace game
  {
    class LoadingThread;
  }

  namespace scene
  {
    class Scene;
    struct SceneComponents;

    enum class LoadingState
    {
      LoadingTextures,
      LoadingSounds
    };

    // The SceneLoader class takes a Stage, which is basically just hard data, and transforms it into a
    // Scene, which makes up the visual and audial game experience.
    class SceneLoader
        : public utility::LoadingInterface<LoadingState>
    {
    public:

      SceneLoader(game::LoadingThread* loading_thread);

      void async_load_scene(const stage::Stage* stage_ptr);

      Scene load(const stage::Stage* stage_ptr);

      Scene get_result();
      bool is_ready() const;

    private:
      boost::optional<std::future<scene::Scene>> scene_future_ = boost::none;
      game::LoadingThread* loading_thread_ = nullptr;
    };

    SceneComponents load_scene_components(const stage::Stage* stage_ptr);
  }
}

#endif