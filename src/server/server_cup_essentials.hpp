/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef SERVER_CUP_ESSENTIALS_HPP_64389118
#define SERVER_CUP_ESSENTIALS_HPP_64389118

#include "server_message_conveyor.hpp"
#include "server_message_dispatcher.hpp"

#include "server_cup_controller.hpp"
#include "server_interaction_host.hpp"

#include "stage/stage_regulator.hpp"
#include "stage/stage_loader.hpp"

#include "race/race.hpp"

namespace ts
{
  namespace resources
  {
    class ResourceStore;
  }

  namespace server
  {
    struct MessageForwarder;

    class CupEssentials
    {
    public:
      explicit CupEssentials(resources::ResourceStore* resource_store);

      void update(std::uint32_t frame_duration);

      const MessageConveyor& message_conveyor() const;
      const cup::Cup& cup() const;
      const stage::Stage* stage() const;

      void advance_cup();
      void async_load_stage(stage::StageDescription&& stage_desc);

      
      void register_local_client(const LocalConveyor* local_conveyor,
                                 const cup::PlayerDefinition* players, std::size_t player_count);

      void handle_ready_signal(const RemoteClient& client);

      template <typename MessageType>
      void forward_stage_message(const ClientMessage<MessageType>& client_message);

    private:
      friend MessageForwarder;
      resources::ResourceStore* resource_store_;

      MessageConveyor message_conveyor_;
      MessageDispatcher message_dispatcher_;

      CupController cup_controller_;
      InteractionHost interaction_host_;

      stage::StageLoader stage_loader_;
      stage::StageRegulator stage_regulator_;      

      std::unique_ptr<race::Race> race_;
    };

    template <typename MessageType>
    void CupEssentials::forward_stage_message(const ClientMessage<MessageType>& client_message)
    {
      if (stage_regulator_)
      {
        stage_regulator_.handle_message(client_message.message);
      }
    }
  }
}

#endif