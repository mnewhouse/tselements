/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CLIENT_ACTION_ESSENTIALS_DETAIL_HPP_28958189234
#define CLIENT_ACTION_ESSENTIALS_DETAIL_HPP_28958189234

#include "client_action_essentials.hpp"
#include "client_cup_essentials.hpp"
#include "local_player_roster.hpp"
#include "key_settings.hpp"
#include "client_action_interface.hpp"

#include "stage/stage.hpp"

#include "resources/resource_store.hpp"
#include "resources/settings.hpp"

#include "world/world_messages.hpp"

namespace ts
{
  namespace client
  {
    namespace detail
    {
      template <typename MessageDispatcher>
      ActionStateDeleter<MessageDispatcher>::ActionStateDeleter(game::StateMachine* state_machine)
        : state_machine_(state_machine)
      {}

      template <typename MessageDispatcher>
      void ActionStateDeleter<MessageDispatcher>::operator()(ActionState<MessageDispatcher>* action_state) const
      {
        state_machine_->destroy_state<ActionState<MessageDispatcher>>();
      }

      template <typename MessageDispatcher>
      auto create_action_state(const CupEssentials<MessageDispatcher>& cup_essentials, 
                               ActionInterface<MessageDispatcher> action_interface)
      {
        using state_type = ActionState<MessageDispatcher>;
        using deleter_type = ActionStateDeleter<MessageDispatcher>;

        auto deferred = components::state_machine::deferred;
        const auto& game_context = cup_essentials.game_context();
        auto action_state = game_context.state_machine->create_state<state_type>(deferred, game_context, action_interface);
        if (action_state == nullptr)
        {
          throw std::logic_error("action state already active");
        }

        return std::unique_ptr<state_type, deleter_type>(action_state, deleter_type(game_context.state_machine));
      }

      const auto& key_mapping(const game::GameContext& game_context)
      {
        return game_context.resource_store->settings().key_settings().key_mapping;
      }
    }

    template <typename MessageDispatcher>
    ActionEssentials<MessageDispatcher>::ActionEssentials(CupEssentials<MessageDispatcher>* cup_essentials, 
                                                          scene::Scene scene_obj)
      : cup_essentials_(cup_essentials),
        scene_(std::move(scene_obj)),
        control_center_(cup_essentials->local_players().create_control_center(scene_.stage_ptr->stage_description())),
        control_event_translator_(scene_.stage_ptr, &control_center_,
                                  detail::key_mapping(cup_essentials->game_context())),
        viewport_arrangement_(make_viewport_arrangement({ 0.0, 0.0, 1.0, 1.0 }, control_center_, *scene_.stage_ptr)),
        action_state_(detail::create_action_state(*cup_essentials, ActionInterface<MessageDispatcher>(this)))
    {}

    template <typename MessageDispatcher>
    void ActionEssentials<MessageDispatcher>::process_event(const game::Event& event)
    {
      control_event_translator_.translate_event(event, cup_essentials_->message_dispatcher());
    }

    template <typename MessageDispatcher>
    void ActionEssentials<MessageDispatcher>::request_update(std::uint32_t frame_duration)
    {
      cup_essentials_->request_update(frame_duration);
    }
    
    template <typename MessageDispatcher>
    void ActionEssentials<MessageDispatcher>::render(const game::RenderContext& render_context) const
    {
      // Render all active viewports through the scene renderer.
      for (std::size_t viewport_id = 0; viewport_id != viewport_arrangement_.viewport_count(); ++viewport_id)
      {
        scene_.scene_renderer.render(viewport_arrangement_.viewport(viewport_id),
                                     render_context.screen_size, render_context.frame_progress);
      }
    }

    template <typename MessageDispatcher>
    void ActionEssentials<MessageDispatcher>::update(std::uint32_t frame_duration)
    {
      // Make sure we update these before the stage update, because these things
      // have to know the previous game state to allow for interpolation.
      viewport_arrangement_.update_viewports();
      update_stored_state(scene_);

      // Request a stage update. This is done through the message system,
      // which delivers the request to the part of the game that's responsible
      // for the stage updates.
      request_update(frame_duration);

      // Now, update the scene with the new game state.
      update_scene(scene_, frame_duration);
    }

    template <typename MessageDispatcher>
    void ActionEssentials<MessageDispatcher>::launch_action()
    {
      // Activate the action state by its type.
      auto state_machine = cup_essentials_->game_context().state_machine;
      state_machine->activate_state<ActionState<MessageDispatcher>>();
    }

    template <typename MessageDispatcher>
    void ActionEssentials<MessageDispatcher>::collision_event(const world::messages::SceneryCollision& event)
    {
      scene::handle_collision(scene_, event);
    }

    template <typename MessageDispatcher>
    void ActionEssentials<MessageDispatcher>::collision_event(const world::messages::EntityCollision& event)
    {
      scene::handle_collision(scene_, event);
    }
  }
}

#endif