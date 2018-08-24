/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "action_state.hpp"
#include "local_player_roster.hpp"
#include "key_settings.hpp"
#include "client_messages.hpp"

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

      inline const auto& key_settings(const game::GameContext& game_context)
      {
        return game_context.resource_store->settings().key_settings();
      }
    }

    ActionState::ActionState(game::GameContext game_context, scene::Scene scene_obj, const LocalPlayerRoster& local_players)
      : GameState(game_context),
      game_context_(game_context),
      key_settings_(detail::key_settings(game_context)),
      scene_(std::move(scene_obj)),
      control_center_(local_players.create_control_center(scene_.stage().stage_description())),
      control_event_translator_(&scene_.stage(), &control_center_, key_settings_.key_mapping),
      viewport_arrangement_(make_viewport_arrangement(detail::default_viewport(*game_context.render_window),
                                                      control_center_, scene_.stage())),
      local_players_(local_players),
      message_dispatcher_()
    {}

    void ActionState::process_event(const event_type& event)
    {
      control_event_translator_.translate_event(event, message_dispatcher_);

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

        else if (event.key.code == sf::Keyboard::Escape)
        {
          end_action();
        }
      }
    }

    void ActionState::request_update(std::uint32_t frame_duration)
    {
      messages::Update message;
      message.frame_duration = frame_duration;

      message_dispatcher_.send(message);
    }
    
    void ActionState::render(const render_context& ctx) const
    {
      auto fp = ctx.frame_progress;
      if (is_paused_) fp = 0.0;

      // Render all active viewports through the scene renderer.
      scene_.render(viewport_arrangement_, ctx.screen_size, fp);
    }

    void ActionState::update(const update_context& ctx)
    {
      if (!is_paused_)
      {
        auto frame_duration = ctx.frame_duration;

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

        if (auto race_tracker = scene_.stage().race_tracker())
        {
          if (hud_visible_)
          {
            race_hud_.update(viewport_arrangement_, *race_tracker);
          }          
        }
      }
    }

    void ActionState::pause()
    {
      is_paused_ = true;

      scene_.pause();
    }

    void ActionState::resume()
    {
      is_paused_ = false;

      scene_.resume();
    }

    void ActionState::toggle_paused()
    {
      if (is_paused_)
      {
        resume();
      }

      else
      {
        pause();
      }
    }

    void ActionState::hide_race_hud()
    {
      hud_visible_ = false;
    }

    void ActionState::show_race_hud()
    {
      hud_visible_ = true;
    }

    void ActionState::connect(server::MessageConveyor local_connection)
    {
      message_dispatcher_.connect(local_connection);

      messages::LocalConnection msg;
      msg.message_conveyor = client::MessageConveyor(this);
      msg.players = local_players_.players();
      msg.player_count = local_players_.player_count();

      message_dispatcher_.send(msg);
    }

    void ActionState::launch_action()
    {
      // Activate the action state by its type.
      game_context_.state_machine->activate_state(typeid(*this));
    }

    void ActionState::end_action()
    {
      game_context_.state_machine->destroy_state(typeid(*this));
    }

    scene::Scene& ActionState::scene_object()
    {
      return scene_;
    }
    
    const scene::Scene& ActionState::scene_object() const
    {
      return scene_;
    }
  }
}
