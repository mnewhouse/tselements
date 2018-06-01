/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

/*

#include "server_message_conveyor.hpp"
#include "server_message_dispatcher.hpp"

#include "server_cup_controller.hpp"
#include "server_interaction_host.hpp"
#include "server_stage.hpp"

#include "stage/stage_loader.hpp"

#include <boost/optional.hpp>

namespace ts
{
  namespace resources
  {
    class ResourceStore;
  }

  namespace server
  {
    struct MessageForwarder;

    class Cup
    {
    public:
      explicit Cup(resources::ResourceStore* resource_store);

      void update(std::uint32_t frame_duration);

      MessageConveyor message_conveyor() const;
      const cup::Cup& cup() const;
      const CupStage* stage() const;

      void register_local_client(const LocalMessageConveyor* local_conveyor,
                                 const cup::PlayerDefinition* players, std::size_t player_count);

      template <typename MessageType>
      void forward_message(MessageType&& message)
      {
        if (stage_)
        {
          stage_->handle_message(message);
        }
      }

    private:
      template <typename MessageType>
      void handle_message(const MessageType&) {}

      void handle_message(cup::messages::PreInitialization&& pre_initialization);
      void handle_message(const ClientMessage<cup::messages::Advance>& advance);
      void handle_message(const ClientMessage<cup::messages::Ready>& ready);
      void handle_message(const ClientMessage<client::messages::Update>& update);

      void advance_cup();
      void async_load_stage(stage::StageDescription&& stage_desc);

      void handle_ready_signal(const RemoteClient& client);

      void initialize_loaded_stage();

      friend MessageForwarder;
      resources::ResourceStore* resource_store_;

      MessageConveyor message_conveyor_;
      MessageDispatcher message_dispatcher_;

      CupController cup_controller_;
      InteractionHost interaction_host_;

      stage::StageLoader stage_loader_;        
      boost::optional<CupStage> stage_;
    };

    template <typename MessageType>
    void forward_message(const MessageContext& ctx, MessageType&& message)
    {
      ctx.cup->forward_message(std::forward<MessageType>(message));
    }
  }
}
*/