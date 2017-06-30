/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "control_event_translator.hpp"
#include "client_viewport_arrangement.hpp"
#include "client_race_hud.hpp"

#include "controls/control_center.hpp"

#include "states/action_state.hpp"

#include "scene/scene.hpp"

#include "world/world_message_fwd.hpp"

#include <memory>

namespace ts
{
  namespace client
  {
    class LocalPlayerRoster;

    template <typename MessageDispatcher>
    class ActionState;

    namespace detail
    {
      // This deleter class simply has the state machine destroy the currently active
      // ActionState<> object, to make for convenient scope-bound destruction.
      template <typename MessageDispatcher>
      class ActionStateDeleter
      {
      public:
        explicit ActionStateDeleter(game::StateMachine* state_machine);

        void operator()(ActionState<MessageDispatcher>* action_state) const;

      private:
        game::StateMachine* state_machine_;
      };

      template <typename MessageDispatcher>
      using ActionStateGuard = std::unique_ptr<ActionState<MessageDispatcher>, ActionStateDeleter<MessageDispatcher>>;
    }

    // The ActionEssentials class template ties the various components required
    // for the "action" part of the game together.
    template <typename MessageDispatcher>
    class ActionEssentials
    {
    public:
      ActionEssentials(game::GameContext game_context, const MessageDispatcher* message_dispatcher,
                       scene::Scene scene_obj, const LocalPlayerRoster& local_player);

      void render(const game::RenderContext& render_context) const;
      void update(std::uint32_t frame_duration);

      void request_update(std::uint32_t frame_duration);
      void process_event(const game::Event& event);

      void launch_action();
      void end_action();

      void collision_event(const world::messages::SceneryCollision& collision);
      void collision_event(const world::messages::EntityCollision& collision);

      scene::Scene& scene_object();
      const scene::Scene& scene_object() const;

    private:
      game::GameContext game_context_;
      const MessageDispatcher* message_dispatcher_;

      KeySettings key_settings_;

      scene::Scene scene_;
      controls::ControlCenter control_center_;
      ControlEventTranslator control_event_translator_;
      scene::ViewportArrangement viewport_arrangement_;

      RaceHUD race_hud_;

      detail::ActionStateGuard<MessageDispatcher> action_state_;
    };
  }
}
