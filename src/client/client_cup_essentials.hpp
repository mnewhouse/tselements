/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CLIENT_CUP_ESSENTIALS_HPP_66819814551
#define CLIENT_CUP_ESSENTIALS_HPP_66819814551

#include "local_player_roster.hpp"
#include "client_message_conveyor.hpp"
#include "client_action_essentials.hpp"

#include "scene/scene_loader.hpp"

#include "game/game_state.hpp"
#include "game/game_context.hpp"

#include <cstdint>

namespace ts
{
  namespace client
  {
    template <typename MessageDispatcher>
    class MessageForwarder;

    // The CupEssentials class template ties all the essential client cup logic together.
    template <typename MessageDispatcher>
    class CupEssentials
    {
    public:
      explicit CupEssentials(const game::GameContext& game_context, MessageDispatcher message_dispatcher);

      void update(std::uint32_t frame_duration);
      void process_event(const game::Event& event);
      void render(const game::RenderContext& render_context) const;

      void request_update(std::uint32_t frame_duration);

      const LocalPlayerRoster& local_players() const;
      const game::GameContext& game_context() const;
      const MessageConveyor<MessageDispatcher>& message_conveyor() const;
      const MessageDispatcher& message_dispatcher() const;

      void async_load_scene(const stage::Stage* stage_ptr);

      void launch_action();
      void end_action();

      void registration_success(std::uint16_t client_id, std::uint64_t client_key);

      ActionEssentials<MessageDispatcher>* action_essentials();
      const ActionEssentials<MessageDispatcher>* action_essentials() const;

    private:
      game::GameContext game_context_;

      MessageDispatcher message_dispatcher_;
      MessageConveyor<MessageDispatcher> message_conveyor_;

      scene::SceneLoader scene_loader_;
      LocalPlayerRoster local_player_roster_;

      std::unique_ptr<ActionEssentials<MessageDispatcher>> action_essentials_;
    };
  }
}

#endif