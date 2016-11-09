/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"
#include "track_editor_test_state.hpp"
#include "test_state_essentials.hpp"

#include "scene/scene_loader.hpp"
#include "scene/scene_components.hpp"
#include "scene/scene.hpp"
#include "scene/render_scene.hpp"

#include "resources/pattern_loader.hpp"
#include "resources/pattern_builder.hpp"
#include "resources/settings.hpp"

#include "cup/cup_settings.hpp"

#include "client/player_settings.hpp"
#include "client/local_player_roster.hpp"

namespace ts
{
  namespace editor
  {
    namespace track
    {
      static auto load_scene_components(const stage::Stage* stage_ptr)
      {
        return std::make_unique<scene::SceneComponents>(scene::load_scene_components(stage_ptr));
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
        auto pattern_loader = preload_pattern_files(track);
        auto pattern = build_track_pattern(track, pattern_loader);

        world::World world_obj(std::move(track), std::move(pattern));

        auto stage_obj = std::make_unique<stage::Stage>(std::move(world_obj),
                                                        make_stage_description(settings, local_players));

        return std::make_unique<test::StageEssentials>(std::move(stage_obj));
      }

      TestState::TestState(resources::Track track, const resources::Settings& settings)
        : local_players_(make_local_player_roster(settings.player_settings())),
          stage_essentials_(load_stage(std::move(track), settings, local_players_)),
          scene_components_(load_scene_components(stage_essentials_->stage()))
      {
      }

      TestState::~TestState() = default;

      void TestState::launch(const game::GameContext& game_context, scene::RenderScene render_scene)
      {
        scene::Scene scene_obj(std::move(*scene_components_), std::move(render_scene));
        test::ClientMessageDispatcher dispatcher(&stage_essentials_->message_conveyor());

        action_essentials_ = std::make_unique<test::ActionEssentials>(game_context, std::move(scene_obj),
                                                                      local_players_, dispatcher);

        stage_essentials_->initiate_local_connection(action_essentials_.get());

        action_essentials_->launch_action();

      }
    }
  }
}