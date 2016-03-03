/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CUP_STATE_BASE_HPP_34131892734
#define CUP_STATE_BASE_HPP_34131892734

#include "action_state.hpp"

#include "client/cup_state_interface.hpp"
#include "client/client_scene_loader.hpp"
#include "client/local_player_controller.hpp"

#include "game/game_state.hpp"

namespace ts
{
  namespace cup
  {
    class Cup;
  }

  namespace client
  {
    template <typename MessageDispatcher>
    class CupStateBase
      : public game::GameState, public CupStateInterface
    {
    public:
      explicit CupStateBase(const game_context& game_context, const MessageDispatcher* message_dispatcher);

      virtual void process_event(const event_type& event) override;
      virtual void update(const update_context&) override;

      virtual void handle_message(const cup::messages::StageBegin&) override;

      virtual const cup::Cup& cup_object() const = 0;

    protected:
      SceneLoader& scene_loader();
      LocalPlayerController& local_player_controller();

    private:
      const MessageDispatcher* message_dispatcher_;
      SceneLoader scene_loader_;
      LocalPlayerController local_player_controller_;

      const ActionState<MessageDispatcher>* action_state_ = nullptr;
    };
  }
}

#endif