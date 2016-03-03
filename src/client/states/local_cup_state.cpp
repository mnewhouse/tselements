/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef LOCAL_CUP_STATE_HPP_339182854
#define LOCAL_CUP_STATE_HPP_339182854

#include "local_cup_state.hpp"
#include "cup_state_base_detail.hpp"

#include "cup/player_definition.hpp"

namespace ts
{
  namespace client
  {
    template class CupStateBase<LocalMessageDispatcher>;

    LocalCupState::LocalCupState(const game_context& game_context)
      : CupStateBase(game_context, &message_dispatcher_),
        server_(game_context.resource_store),
        message_dispatcher_(&server_.message_conveyor()),
        message_conveyor_(make_message_context())
    {
      cup::PlayerDefinition player_def;
      player_def.id = 0xBEEF;
      player_def.name = "Tinjowitsu";
      player_def.control_slot = 0;

      local_player_controller() = LocalPlayerController(&player_def, 1);

      server_.initiate_local_connection(&message_conveyor_, &player_def, 1);
    }

    void LocalCupState::update(const update_context& context)
    {
      CupStateBase<LocalMessageDispatcher>::update(context);

      server_.update(context.frame_duration);
    }

    MessageContext LocalCupState::make_message_context()
    {
      MessageContext context;
      context.cup_synchronizer = nullptr;
      context.scene_loader = &scene_loader();
      context.local_player_controller = &local_player_controller();
      context.cup_state_interface = this;
      return context;
    }

    const cup::Cup& LocalCupState::cup_object() const
    {
      return server_.cup();
    }
  }
}

#endif