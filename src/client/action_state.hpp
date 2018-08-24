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

      scene::Scene& scene_object();
      const scene::Scene& scene_object() const;

      void connect(server::MessageConveyor message_conveyor);

      void pause();
      void resume();
      void toggle_paused();
      bool is_paused() const { return is_paused_; }
    
      const controls::ControlCenter& control_center() const { return control_center_; }
      const scene::Scene& scene_obj() const { return scene_; }

      void hide_race_hud();
      void show_race_hud();

      //template <typename MessageType>
      //void handle_message(const MessageType&) {}

      //void handle_message(const world::messages::SceneryCollision& collision);
      //void handle_message(const world::messages::EntityCollision& collision);
    protected:
      template <typename MessageType>
      void dispatch_message(const MessageType& m)
      {
        message_dispatcher_.send(m);
      }

    private:
      virtual void launch_action();
      virtual void end_action();

      game::GameContext game_context_;

      KeySettings key_settings_;

      scene::Scene scene_;
      controls::ControlCenter control_center_;
      ControlEventTranslator control_event_translator_;
      scene::ViewportArrangement viewport_arrangement_;
      LocalPlayerRoster local_players_;

      RaceHUD race_hud_;
      bool is_paused_ = false;
      bool hud_visible_ = true;

      MessageDispatcher message_dispatcher_;
    };
  }
}
