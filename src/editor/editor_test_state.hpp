/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "client/local_player_roster.hpp"
#include "client/standalone_action_state.hpp"

#include "game/game_context.hpp"

#include <functional>

namespace ts
{
  namespace resources
  {
    class Track;
    class Settings;
  }

  namespace scene
  {
    class RenderScene;
  }

  namespace editor
  {
    class EditorScene;

    struct StageComponents
    {
      StageComponents();
      ~StageComponents();

      StageComponents(StageComponents&&);
      StageComponents& operator=(StageComponents&&);

      client::LocalPlayerRoster local_players;
      std::unique_ptr<stage::Stage> stage;
      std::unique_ptr<scene::SceneComponents> scene_components;      
    };

    void adopt_render_scene(StageComponents& stage_components, scene::RenderScene render_scene);
    StageComponents load_test_stage_components(resources::Track track, const resources::Settings& settings);

    class TestState
      : public client::StandaloneActionState
    {
    public:
      TestState(const game_context& ctx, StageComponents stage_components);

      void release_scene(EditorScene& editor_scene);    
    };
  }
}
