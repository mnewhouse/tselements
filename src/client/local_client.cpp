/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "local_client.hpp"
#include "client_base_detail.hpp"
#include "client_cup_essentials_detail.hpp"
#include "client_action_essentials_detail.hpp"
#include "client_action_interface_detail.hpp"
#include "client_message_forwarder_detail.hpp"
#include "control_event_translator_detail.hpp"
#include "local_player_roster.hpp"

namespace ts
{
  namespace client
  {
    // Explicitly instantiate the templates we're going to need for local client things.
    template class ClientBase<LocalMessageDispatcher>;
    template class CupEssentials<LocalMessageDispatcher>;

    template class ActionEssentials<LocalMessageDispatcher>;
    template class detail::ActionStateDeleter<LocalMessageDispatcher>;

    template class ActionInterface<LocalMessageDispatcher>;
    template class MessageForwarder<LocalMessageDispatcher>;
    template class ActionMessageForwarder<LocalMessageDispatcher>;

    template void ControlEventTranslator::translate_event(const game::Event& event, const LocalMessageDispatcher& message_dispatcher) const;

    LocalClient::LocalClient(const game::GameContext& game_context)
      : server::Server(game_context.resource_store),
        ClientBase(game_context, LocalMessageDispatcher(&Server::message_conveyor()))
    {
      const auto& players = local_players();
      register_local_client(&ClientBase::message_conveyor(), players.players(), players.player_count());
    }

    void LocalClient::update(std::uint32_t frame_duration)
    {
      Server::update(frame_duration);
      ClientBase::update(frame_duration);
    }
  }
}