/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef TEST_STATE_ESSENTIALS_HPP_583745783
#define TEST_STATE_ESSENTIALS_HPP_583745783

#include "messages/message_conveyor.hpp"

#include "client/client_action_essentials.hpp"

#include "server/server_stage_essentials.hpp"

namespace ts
{
  namespace editor
  {
    namespace test
    {
      class StageEssentials;
      class ActionEssentials;

      struct ServerMessageContext
      {
        StageEssentials* stage_essentials_;
      };

      using ServerMessageConveyor = messages::MessageConveyor<ServerMessageContext>;

      // The following class dispatches the outgoing control events to the server.
      class ClientMessageDispatcher
      {
      public:
        explicit ClientMessageDispatcher(const ServerMessageConveyor* conveyor);

        template <typename MessageType>
        void operator()(const MessageType& message) const
        {
          (*conveyor_)(server::make_client_message(message, server::local_client));
        }

      private:
        const ServerMessageConveyor* conveyor_;
      };

      // This thing dispatches the game events that are sent from the server side.
      class ServerMessageDispatcher
      {
      public:
        explicit ServerMessageDispatcher(ActionEssentials* action_essentials)
          : action_essentials_(action_essentials)
        {
        }

        template <typename MessageType, typename ClientType>
        void operator()(const MessageType& message, const ClientType& client) const
        {
          // Send to client
          client::ActionMessageForwarder<ClientMessageDispatcher> forwarder(action_essentials_);

          forwarder.forward(message);
        }

      private:
        ActionEssentials* action_essentials_;
      };

      class ActionEssentials
        : public client::ActionEssentials<ClientMessageDispatcher>
      {
      public:
        using base_type = client::ActionEssentials<ClientMessageDispatcher>;
        ActionEssentials(game::GameContext game_context, scene::Scene scene_obj,
                         const client::LocalPlayerRoster& local_players, ClientMessageDispatcher message_dispatcher);

      private:
        ClientMessageDispatcher message_dispatcher_;
      };

      class StageEssentials
        : public server::StageEssentials<ServerMessageDispatcher, ServerMessageConveyor>
      {
      public:
        using base_type = server::StageEssentials<ServerMessageDispatcher, ServerMessageConveyor>;
        StageEssentials(std::unique_ptr<stage::Stage> stage);

        const ServerMessageConveyor& message_conveyor() const;

        void initiate_local_connection(ActionEssentials* action_essentials);

      private:
        ServerMessageDispatcher message_dispatcher_;
        ServerMessageConveyor message_conveyor_;
      };

      template <typename MessageType>
      auto forward_message(const ServerMessageContext& context, const MessageType& message, int)
        -> decltype(context.stage_essentials_->handle_message(message))
      {
        return context.stage_essentials_->handle_message(message);
      }

      template <typename MessageType>
      void forward_message(const ServerMessageContext& context, const MessageType& message, short)
      {
      }

      template <typename MessageType>
      void forward_message(const ServerMessageContext& context, const MessageType& message)
      {
        forward_message(context, message, 0);
      }
    }
  }
}

#endif