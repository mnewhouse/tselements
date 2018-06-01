/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

/*

#include "client_cup.hpp"
#include "update_message.hpp"
#include "player_settings.hpp"

#include "states/action_state.hpp"

#include "cup/cup_messages.hpp"

#include "stage/stage_messages.hpp"

#include "resources/resource_store.hpp"
#include "resources/settings.hpp"

#include <memory>
#include <exception>
#include <iostream>

namespace ts
{
  namespace client
  {
    namespace detail
    {
      template <typename MessageDispatcher>
      auto make_message_context(Cup<MessageDispatcher>* cup)
      {
        MessageContext<MessageDispatcher> context;
        context.cup = cup;
        return context;
      }

      static auto make_local_player_roster(const game::GameContext& game_context)
      {
        const auto& player_settings = game_context.resource_store->settings().player_settings();
        const auto& selected_players = player_settings.selected_players;
        return LocalPlayerRoster(selected_players.data(), selected_players.size());
      }
    }

    template <typename MessageDispatcher>
    Cup<MessageDispatcher>::Cup(const game::GameContext& game_context,
                                          MessageDispatcher message_dispatcher)
      : game_context_(game_context),
        message_conveyor_(detail::make_message_context(this)),
        message_dispatcher_(std::move(message_dispatcher)),
        scene_loader_(game_context.loading_thread),
        local_player_roster_(detail::make_local_player_roster(game_context))
    {}

    template <typename MessageDispatcher>
    void Cup<MessageDispatcher>::process_event(const game::Event& event)
    {
      if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Return)
      {
        message_dispatcher_(cup::messages::Advance());
      }
    }

    template <typename MessageDispatcher>
    void Cup<MessageDispatcher>::request_update(std::uint32_t frame_duration)
    {
      messages::Update message;
      message.frame_duration = frame_duration;

      message_dispatcher_(message);
    }

    template <typename MessageDispatcher>
    void Cup<MessageDispatcher>::update(std::uint32_t frame_duration)
    {
      if (scene_loader_.is_loading() && scene_loader_.is_ready())
      {
        try
        {
          // When scene loading is finished, send ready message
          using action_type = Action<MessageDispatcher>;
          action_ = std::make_unique<action_type>(game_context_, message_dispatcher_,
                                                  scene_loader_.get_result(), local_player_roster_);
        }

        catch (const std::exception& e)
        {
          // If something went wrong with the loading process, handle it gracefully #TODO
          std::cout << e.what() << std::endl;
        }

        // Inform the server that we are ready, regardless of whether an exception happened.
        message_dispatcher_(cup::messages::Ready());
      }
    }

    template <typename MessageDispatcher>
    void Cup<MessageDispatcher>::render(const game::RenderContext& render_context) const
    {
    }

    template <typename MessageDispatcher>
    const LocalPlayerRoster& Cup<MessageDispatcher>::local_players() const
    {
      return local_player_roster_;
    }

    template <typename MessageDispatcher>
    const game::GameContext& Cup<MessageDispatcher>::game_context() const
    {
      return game_context_;
    }

    template <typename MessageDispatcher>
    const MessageConveyor<MessageDispatcher>& Cup<MessageDispatcher>::message_conveyor() const
    {
      return message_conveyor_;
    }

    template <typename MessageDispatcher>
    const MessageDispatcher& Cup<MessageDispatcher>::message_dispatcher() const
    {
      return message_dispatcher_;
    }

    template <typename MessageDispatcher>
    void Cup<MessageDispatcher>::async_load_scene(const stage::Stage* stage)
    {
      scene_loader_.async_load_scene(stage);
    }

    template <typename MessageDispatcher>
    void Cup<MessageDispatcher>::launch_action()
    {
      if (action_)
      {
        action_->launch_action();
      }
    }

    template <typename MessageDispatcher>
    void Cup<MessageDispatcher>::end_action()
    {
      action_.reset();
    }

    template <typename MessageDispatcher>
    void Cup<MessageDispatcher>::registration_success(std::uint16_t client_id, std::uint64_t client_key)
    {
      local_player_roster_.registration_success(client_id);
    }

    template <typename MessageDispatcher>
    Action<MessageDispatcher>* Cup<MessageDispatcher>::action()
    {
      return action_.get();
    }

    template <typename MessageDispatcher>
    const Action<MessageDispatcher>* Cup<MessageDispatcher>::action() const
    {
      return action_.get();
    }

    template <typename MessageDispatcher>
    void Cup<MessageDispatcher>::handle_message(const cup::messages::StageEnd&)
    {
      end_action();
    }

    template <typename MessageDispatcher>
    void Cup<MessageDispatcher>::handle_message(const cup::messages::StageBegin&)
    {
      launch_action();
    }

    template <typename MessageDispatcher>
    void Cup<MessageDispatcher>::handle_message(const stage::messages::StageLoaded& m)
    {
      async_load_scene(m.stage_ptr);
    }

    template <typename MessageDispatcher>
    void Cup<MessageDispatcher>::handle_message(const cup::messages::RegistrationSuccess& m)
    {
      registration_success(m.client_id, m.client_key);
    }
  }
}
*/