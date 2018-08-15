/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "editor_test_state.hpp"
#include "editor_scene.hpp"

#include "scene/scene_loader.hpp"
#include "scene/scene_components.hpp"
#include "scene/scene.hpp"
#include "scene/render_scene.hpp"

#include "resources/settings.hpp"

#include "cup/cup_settings.hpp"

#include "client/player_settings.hpp"
#include "client/local_player_roster.hpp"

#include "world/terrain_map_builder.hpp"
#include "world/terrain_map.hpp"

namespace ts
{
  namespace editor
  {
    StageComponents::StageComponents() = default;
    StageComponents::~StageComponents() = default;
    StageComponents::StageComponents(StageComponents&&) = default;
    StageComponents& StageComponents::operator=(StageComponents&&) = default;

    static auto load_scene_components(const stage::Stage* stage_ptr)
    {
      return std::make_unique<scene::SceneComponents>(scene::load_scene_components_no_render(stage_ptr));
    }

    static auto make_local_player_roster(const client::PlayerSettings& player_settings)
    {
      client::LocalPlayerRoster local_players(player_settings.selected_players.data(),
                                              player_settings.selected_players.size());

      local_players.registration_success(0);
      return local_players;
    }

    static auto make_stage_description(const resources::Settings& settings,
                                       const client::LocalPlayerRoster& local_players)
    {
      const auto& cup_settings = settings.cup_settings();
      const auto& player_settings = settings.player_settings();

      stage::StageDescription stage_description;
      if (!cup_settings.selected_cars.empty())
      {
        stage_description.car_models.push_back(cup_settings.selected_cars.front());

        const auto players = local_players.players();
        for (std::uint8_t slot_id = 0; slot_id != local_players.player_count(); ++slot_id)
        {
          const auto& selected_player = players[slot_id];

          stage::object_description::Car car;
          car.instance_id = slot_id;
          car.model_id = 0;
          car.controller_id = 0; // Client id
          car.slot_id = slot_id;
          car.start_pos = slot_id;
          car.color_scheme = selected_player.color_scheme;

          stage_description.car_instances.push_back(car);
        }
      }

      return stage_description;
    }

    static auto load_stage(resources::Track track, const resources::Settings& settings,
                           const client::LocalPlayerRoster& local_players)
    {
      auto terrain_map = world::build_terrain_map(track);

      world::World world_obj(std::move(track), std::move(terrain_map));

      return std::make_unique<stage::Stage>(std::move(world_obj),
                                                      make_stage_description(settings, local_players));
    }

    StageComponents load_test_stage_components(resources::Track track, const resources::Settings& settings)
    {
      auto terrain_map = world::build_terrain_map(track);
      world::World world_obj(std::move(track), std::move(terrain_map));

      StageComponents result;
      result.local_players = make_local_player_roster(settings.player_settings());
      result.stage = std::make_unique<stage::Stage>(std::move(world_obj), make_stage_description(settings, result.local_players));
      result.scene_components = load_scene_components(result.stage.get());
      return result;
    }

    void adopt_render_scene(StageComponents& stage_components, scene::RenderScene render_scene)
    {
      stage_components.scene_components->render_scene_ = std::move(render_scene);
    }

    TestState::TestState(const game_context& ctx, StageComponents stage_components)
      : client::StandaloneActionState(ctx, scene::Scene(std::move(*stage_components.scene_components)),
                                      std::move(stage_components.local_players),
                                      std::move(stage_components.stage))
    {
    }

    void TestState::release_scene(EditorScene& editor_scene)
    {
      editor_scene.adopt_render_scene(scene_object().release().render_scene_);
    }
  }
}
