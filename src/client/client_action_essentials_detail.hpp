/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CLIENT_ACTION_ESSENTIALS_DETAIL_HPP_28958189234
#define CLIENT_ACTION_ESSENTIALS_DETAIL_HPP_28958189234

#include "client_action_essentials.hpp"
#include "local_player_roster.hpp"
#include "key_settings.hpp"
#include "client_action_interface.hpp"

#include "graphics/render_window.hpp"

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
      inline auto default_viewport(const graphics::RenderWindow& render_window)
      {
        auto window_size = render_window.size();
        return IntRect(0, 0, window_size.x, window_size.y);
      }

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
      auto create_action_state(const game::GameContext& game_context,
                               ActionInterface<MessageDispatcher> action_interface)
      {
        using state_type = ActionState<MessageDispatcher>;
        using deleter_type = ActionStateDeleter<MessageDispatcher>;

        auto deferred = components::state_machine::deferred;
        auto action_state = game_context.state_machine->create_state<state_type>(deferred, game_context, action_interface);
        if (action_state == nullptr)
        {
          throw std::logic_error("action state already active");
        }

        return std::unique_ptr<state_type, deleter_type>(action_state, deleter_type(game_context.state_machine));
      }

      inline const auto& key_settings(const game::GameContext& game_context)
      {
        return game_context.resource_store->settings().key_settings();
      }
    }

    template <typename MessageDispatcher>
    ActionEssentials<MessageDispatcher>::ActionEssentials(game::GameContext game_context,
                                                          const MessageDispatcher* message_dispatcher,
                                                          scene::Scene scene_obj,
                                                          const LocalPlayerRoster& local_players)
      : game_context_(game_context),
        message_dispatcher_(message_dispatcher),
        key_settings_(detail::key_settings(game_context)),
        scene_(std::move(scene_obj)),
        control_center_(local_players.create_control_center(scene_.stage().stage_description())),
        control_event_translator_(&scene_.stage(), &control_center_, key_settings_.key_mapping),
        viewport_arrangement_(make_viewport_arrangement(detail::default_viewport(*game_context.render_window), 
                                                        control_center_, scene_.stage())),
        action_state_(detail::create_action_state(game_context, ActionInterface<MessageDispatcher>(this)))
    {}

    template <typename MessageDispatcher>
    void ActionEssentials<MessageDispatcher>::process_event(const game::Event& event)
    {
      control_event_translator_.translate_event(event, *message_dispatcher_);

      if (event.type == sf::Event::KeyReleased)
      {        
        auto do_zoom = [&](double factor)
        {
          for (std::size_t id = 0; id != viewport_arrangement_.viewport_count(); ++id)
          {
            auto& camera = viewport_arrangement_.viewport(id).camera();
            auto zoom = std::min(std::max(camera.zoom_level() * factor, 0.2), 5.0);
            camera.set_zoom_level(zoom);
          }
        };

        const double zoom_factor = 1.05;
        if (event.key.code == key_settings_.zoom_in_key)
        {
          do_zoom(zoom_factor);
        }

        else if (event.key.code == key_settings_.zoom_out_key)
        {
          do_zoom(1.0 / zoom_factor);
        }
      }
    }

    template <typename MessageDispatcher>
    void ActionEssentials<MessageDispatcher>::request_update(std::uint32_t frame_duration)
    {
      messages::Update message;
      message.frame_duration = frame_duration;

      (*message_dispatcher_)(message);
    }
    
    template <typename MessageDispatcher>
    void ActionEssentials<MessageDispatcher>::render(const game::RenderContext& render_context) const
    {
      // Render all active viewports through the scene renderer.
      scene_.render(viewport_arrangement_, render_context.screen_size, render_context.frame_progress);      
    }

    template <typename MessageDispatcher>
    void ActionEssentials<MessageDispatcher>::update(std::uint32_t frame_duration)
    {
      // Make sure we update these before the stage update, because these things
      // have to know the previous game state to allow for interpolation.
      viewport_arrangement_.update_viewports();
      scene_.update_stored_state();

      // Request a stage update. This is done through the message system,
      // which delivers the request to the part of the game that's responsible
      // for the stage updates.
      request_update(frame_duration);

      // Now, update the scene with the new game state.
      scene_.update(frame_duration);
    }

    template <typename MessageDispatcher>
    void ActionEssentials<MessageDispatcher>::launch_action()
    {
      // Activate the action state by its type.
      game_context_.state_machine->activate_state<ActionState<MessageDispatcher>>();
    }

    template <typename MessageDispatcher>
    void ActionEssentials<MessageDispatcher>::collision_event(const world::messages::SceneryCollision& event)
    {
      scene_.handle_collision(event);
    }

    template <typename MessageDispatcher>
    void ActionEssentials<MessageDispatcher>::collision_event(const world::messages::EntityCollision& event)
    {
      scene_.handle_collision(event);
    }
  }
}

#endif