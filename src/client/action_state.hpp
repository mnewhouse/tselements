/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "game/game_state.hpp"

#include "local_player_roster.hpp"
#include "client_message_dispatcher.hpp"
#include "control_event_translator.hpp"
#include "client_viewport_arrangement.hpp"
#include "client_race_hud.hpp"

#include "controls/control_center.hpp"

#include "scene/scene.hpp"

#include "world/world_message_fwd.hpp"

#include <memory>

namespace ts
{
  namespace client
  {
    class ActionState
      : public game::GameState
    {
    public:
      ActionState(game::GameContext game_context, scene::Scene scene_obj, const LocalPlayerRoster& local_player);

      virtual void render(const render_context&) const override;
      virtual void update(const update_context&) override;
      virtual void process_event(const event_type&) override;

      void request_update(std::uint32_t frame_duration);

      void launch_action();
      void end_action();

      scene::Scene& scene_object();
      const scene::Scene& scene_object() const;

      void connect(server::MessageConveyor message_conveyor);

      //template <typename MessageType>
      //void handle_message(const MessageType&) {}

      //void handle_message(const world::messages::SceneryCollision& collision);
      //void handle_message(const world::messages::EntityCollision& collision);

    private:
      game::GameContext game_context_;

      KeySettings key_settings_;

      scene::Scene scene_;
      controls::ControlCenter control_center_;
      ControlEventTranslator control_event_translator_;
      scene::ViewportArrangement viewport_arrangement_;
      LocalPlayerRoster local_players_;

      RaceHUD race_hud_;

      MessageDispatcher message_dispatcher_;
    };
  }
}
