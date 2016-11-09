/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef TRACK_EDITOR_TEST_SCENE_HPP_58192835
#define TRACK_EDITOR_TEST_SCENE_HPP_58192835

#include "stage/stage_regulator.hpp"

#include "game/game_context.hpp"

#include "client/local_player_roster.hpp"

namespace ts
{
  namespace resources
  {
    class Track;
    class Settings;
  }

  namespace scene
  {
    struct SceneComponents;
    class RenderScene;
  }

  namespace editor
  {
    namespace test
    {
      class ActionEssentials;
      class StageEssentials;
    }

    namespace track
    {
      class TestState
      {
      public:
        explicit TestState(resources::Track track, const resources::Settings& settings);
        ~TestState();

        TestState(TestState&&);
        TestState& operator=(TestState&&);

        void launch(const game::GameContext& game_context, scene::RenderScene render_scene);

      private:
        client::LocalPlayerRoster local_players_;
        std::unique_ptr<test::StageEssentials> stage_essentials_;
        std::unique_ptr<scene::SceneComponents> scene_components_;
        std::unique_ptr<test::ActionEssentials> action_essentials_;        
      };
    }
  }
}

#endif