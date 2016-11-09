/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"
#include "test_state_essentials.hpp"

#include "client/states/action_state_detail.hpp"
#include "client/client_action_essentials_detail.hpp"
#include "client/client_action_interface_detail.hpp"
#include "client/control_event_translator_detail.hpp"
#include "client/client_action_message_forwarder_detail.hpp"

#include "server/server_stage_essentials_detail.hpp"

namespace ts
{
  namespace server
  {
    template class StageEssentials<editor::test::ServerMessageDispatcher,
      editor::test::ServerMessageConveyor>;
  }

  namespace client
  {
    using editor::test::ClientMessageDispatcher;
    using editor::test::ServerMessageDispatcher;

    template class ActionMessageForwarder<ServerMessageDispatcher>;

    template class ActionEssentials<ClientMessageDispatcher>;
    template class detail::ActionStateDeleter<ClientMessageDispatcher>;
    template class ActionInterface<ClientMessageDispatcher>;
    template void ControlEventTranslator::translate_event(const game::Event& event, 
                                                          const ClientMessageDispatcher& message_dispatcher) const;
  }

  namespace editor
  {
    namespace test
    {
      ClientMessageDispatcher::ClientMessageDispatcher(const ServerMessageConveyor* conveyor)
        : conveyor_(conveyor)
      {}

      ActionEssentials::ActionEssentials(game::GameContext game_context, scene::Scene scene_obj,
                                         const client::LocalPlayerRoster& local_players, 
                                         ClientMessageDispatcher message_dispatcher)
        : base_type(game_context, &message_dispatcher_, std::move(scene_obj), local_players),
          message_dispatcher_(std::move(message_dispatcher))
      {
      }

      StageEssentials::StageEssentials(std::unique_ptr<stage::Stage> stage_obj)
        : base_type(std::move(stage_obj), &message_dispatcher_, &message_conveyor_),
          message_dispatcher_(nullptr),
          message_conveyor_(ServerMessageContext{ this })
      {
      }

      void StageEssentials::initiate_local_connection(ActionEssentials* action_essentials)
      {
        message_dispatcher_ = ServerMessageDispatcher(action_essentials);
      }

      const ServerMessageConveyor& StageEssentials::message_conveyor() const
      {
        return message_conveyor_;
      }
    }
  }
}
