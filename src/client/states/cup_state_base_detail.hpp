/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CUP_STATE_BASE_DETAIL_HPP_338129812893
#define CUP_STATE_BASE_DETAIL_HPP_338129812893

#include "cup_state_base.hpp"

#include "client/control_event_translator.hpp"

#include "controls/control_center.hpp"

#include "cup/cup.hpp"
#include "cup/cup_messages.hpp"

#include "stage/stage.hpp"

namespace ts
{
  namespace client
  {
    template <typename MessageDispatcher>
    CupStateBase<MessageDispatcher>::
      CupStateBase(const game_context& game_context, 
                   const MessageDispatcher* message_dispatcher)
      : game::GameState(game_context),
        message_dispatcher_(message_dispatcher),
        scene_loader_(game_context.loading_thread),
        local_player_controller_(nullptr, 0)
    {
    }

    template <typename MessageDispatcher>
    void CupStateBase<MessageDispatcher>::
      process_event(const event_type& event)
    {
      if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Return)
      {
        (*message_dispatcher_)(cup::messages::Advance());
      }
    }
    
    template <typename MessageDispatcher>
    void CupStateBase<MessageDispatcher>::
      update(const update_context& update_context)
    {
      if (scene_loader_.is_loading())
      {
        // If scene loading is finished, send ready message
        if (scene_loader_.is_ready())
        {
          auto scene = scene_loader_.get_result();
          const auto& stage_desc = scene.stage_ptr->stage_description();

          auto control_center = local_player_controller_.create_control_center(stage_desc);
          
          using action_state = ActionState<MessageDispatcher>;
          auto state_machine = context().state_machine;

          UpdateInterface update_interface(this);

          state_machine->destroy_state<action_state>();
          action_state_ = state_machine->create_state<action_state>(context(), std::move(scene),
                                                                    std::move(control_center), 
                                                                    message_dispatcher_,
                                                                    update_interface);

          (*message_dispatcher_)(cup::messages::Ready());
        }
      }
    }

    template <typename MessageDispatcher>
    SceneLoader& CupStateBase<MessageDispatcher>::scene_loader()
    {
      return scene_loader_;
    }

    template <typename MessageDispatcher>
    LocalPlayerController& CupStateBase<MessageDispatcher>::
      local_player_controller()
    {
      return local_player_controller_;
    }

    template <typename MessageDispatcher>
    void CupStateBase<MessageDispatcher>::
      handle_message(const cup::messages::StageBegin&)
    {
      if (action_state_)
      {
        context().state_machine->activate_state<ActionState<MessageDispatcher>>();
        action_state_ = nullptr;
      }
    }
  }
}

#endif