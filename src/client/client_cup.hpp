/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once
/*

#include "local_player_roster.hpp"
#include "client_action.hpp"

#include "messages/message_conveyor.hpp"

#include "cup/cup_message_fwd.hpp"
#include "stage/stage_message_fwd.hpp"
#include "stage/race_message_fwd.hpp"

#include "scene/scene_loader.hpp"

#include "game/game_state.hpp"
#include "game/game_context.hpp"

#include <cstdint>

namespace ts
{
  namespace client
  {    
    class Cup
    {
    public:
      explicit Cup(const game::GameContext& game_context, MessageDispatcher message_dispatcher);

      void update(std::uint32_t frame_duration);
      void process_event(const game::Event& event);
      void render(const game::RenderContext& render_context) const;

      void request_update(std::uint32_t frame_duration);

      const LocalPlayerRoster& local_players() const;
      const game::GameContext& game_context() const;
      const MessageConveyor<MessageDispatcher>& message_conveyor() const;
      const MessageDispatcher& message_dispatcher() const;

      void registration_success(std::uint16_t client_id, std::uint64_t client_key);

      Action* action();
      const Action* action() const;

      template <typename MessageDispatcher, typename MessageType>
      friend void forward_message(const MessageContext<MessageDispatcher>& context, const MessageType& message)
      {
        context.cup->handle_message(message);
      }

    private:      
      template <typename MessageType>
      void handle_message(const MessageType&) {}

      void handle_message(const cup::messages::RegistrationSuccess&);
      void handle_message(const stage::messages::StageLoaded&);
      void handle_message(const cup::messages::StageBegin&);
      void handle_message(const cup::messages::StageEnd&);

      void async_load_scene(const stage::Stage* stage_ptr);

      void launch_action();
      void end_action();

      game::GameContext game_context_;

      MessageDispatcher message_dispatcher_;
      MessageConveyor<MessageDispatcher> message_conveyor_;

      scene::SceneLoader scene_loader_;
      LocalPlayerRoster local_player_roster_;

      std::unique_ptr<Action<MessageDispatcher>> action_;
    };
  }
}
*/
