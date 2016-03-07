/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CLIENT_CUP_ESSENTIALS_DETAIL_HPP_5819852193859
#define CLIENT_CUP_ESSENTIALS_DETAIL_HPP_5819852193859

#include "client_cup_essentials.hpp"
#include "update_message.hpp"
#include "player_settings.hpp"

#include "states/action_state.hpp"

#include "cup/cup_messages.hpp"

#include "resources/resource_store.hpp"
#include "resources/settings.hpp"

#include <memory>
#include <exception>

namespace ts
{
  namespace client
  {
    namespace detail
    {
      template <typename MessageDispatcher>
      auto make_message_context(CupEssentials<MessageDispatcher>* cup_essentials)
      {
        MessageContext<MessageDispatcher> context;
        context.cup_essentials = cup_essentials;
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
    CupEssentials<MessageDispatcher>::CupEssentials(const game::GameContext& game_context,
                                                    MessageDispatcher message_dispatcher)
      : game_context_(game_context),
        message_conveyor_(detail::make_message_context(this)),
        message_dispatcher_(std::move(message_dispatcher)),
        scene_loader_(game_context.loading_thread),
        local_player_roster_(detail::make_local_player_roster(game_context))
    {}

    template <typename MessageDispatcher>
    void CupEssentials<MessageDispatcher>::process_event(const game::Event& event)
    {
      if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Return)
      {
        message_dispatcher_(cup::messages::Advance());
      }
    }

    template <typename MessageDispatcher>
    void CupEssentials<MessageDispatcher>::request_update(std::uint32_t frame_duration)
    {
      messages::Update message;
      message.frame_duration = frame_duration;

      message_dispatcher_(message);
    }

    template <typename MessageDispatcher>
    void CupEssentials<MessageDispatcher>::update(std::uint32_t frame_duration)
    {
      request_update(frame_duration);

      if (scene_loader_.is_loading() && scene_loader_.is_ready())
      {
        try
        {
          // When scene loading is finished, send ready message
          using action_type = ActionEssentials<MessageDispatcher>;
          action_essentials_ = std::make_unique<action_type>(this, scene_loader_.get_result());
        }

        catch (const std::exception&)
        {
          // If something went wrong with the loading process, handle it gracefully #TODO
        }

        // Inform the server that we are ready, regardless of whether an exception happened.
        message_dispatcher_(cup::messages::Ready());
      }
    }

    template <typename MessageDispatcher>
    void CupEssentials<MessageDispatcher>::render(const game::RenderContext& render_context) const
    {
    }

    template <typename MessageDispatcher>
    const LocalPlayerRoster& CupEssentials<MessageDispatcher>::local_players() const
    {
      return local_player_roster_;
    }

    template <typename MessageDispatcher>
    const game::GameContext& CupEssentials<MessageDispatcher>::game_context() const
    {
      return game_context_;
    }

    template <typename MessageDispatcher>
    const MessageConveyor<MessageDispatcher>& CupEssentials<MessageDispatcher>::message_conveyor() const
    {
      return message_conveyor_;
    }

    template <typename MessageDispatcher>
    const MessageDispatcher& CupEssentials<MessageDispatcher>::message_dispatcher() const
    {
      return message_dispatcher_;
    }

    template <typename MessageDispatcher>
    void CupEssentials<MessageDispatcher>::async_load_scene(const stage::Stage* stage)
    {
      scene_loader_.async_load_scene(stage);
    }

    template <typename MessageDispatcher>
    void CupEssentials<MessageDispatcher>::launch_action()
    {
      if (action_essentials_)
      {
        action_essentials_->launch_action();
      }
    }

    template <typename MessageDispatcher>
    void CupEssentials<MessageDispatcher>::end_action()
    {
      action_essentials_.reset();
    }

    template <typename MessageDispatcher>
    void CupEssentials<MessageDispatcher>::registration_success(std::uint16_t client_id, std::uint64_t client_key)
    {
      local_player_roster_.registration_success(client_id);
    }

    template <typename MessageDispatcher>
    ActionEssentials<MessageDispatcher>* CupEssentials<MessageDispatcher>::action_essentials()
    {
      return action_essentials_.get();
    }

    template <typename MessageDispatcher>
    const ActionEssentials<MessageDispatcher>* CupEssentials<MessageDispatcher>::action_essentials() const
    {
      return action_essentials_.get();
    }
  }
}



#endif