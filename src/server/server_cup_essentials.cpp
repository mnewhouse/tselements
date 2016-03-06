/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "server_cup_essentials.hpp"
#include "server_message_distributor.hpp"

#include "resources/resource_store.hpp"
#include "resources/settings.hpp"

#include "world/world_event_translator_detail.hpp"

namespace ts
{
  namespace server
  {
    namespace detail
    {
      auto make_message_distributor(const MessageDispatcher* dispatcher, const MessageConveyor* conveyor)
      {
        MessageDistributor distributor(dispatcher, conveyor);
        return world::EventTranslator<MessageDistributor>(std::move(distributor));
      }
    }

    CupEssentials::CupEssentials(resources::ResourceStore* resource_store)
      : resource_store_(resource_store),
      message_conveyor_(MessageContext{ this }),
      message_dispatcher_(),
      cup_controller_(resource_store->settings().cup_settings(),
                      MessageDistributor(&message_dispatcher_, &message_conveyor_)),
      interaction_host_(&cup_controller_, &message_dispatcher_),
      stage_regulator_(),
      stage_loader_(),
      race_()
    {}

    void CupEssentials::update(std::uint32_t frame_duration)
    {
      if (stage_regulator_.active())
      {        
        stage_regulator_.update(detail::make_message_distributor(&message_dispatcher_, &message_conveyor_), 
                                frame_duration);
      }

      if (stage_loader_.is_ready())
      {
        try
        {
          stage_regulator_.adopt_stage(stage_loader_.get_result());

          const auto& stage_desc = stage_regulator_.stage()->stage_description();
          cup_controller_.initialize_stage(stage_desc);

          stage::messages::StageLoaded stage_loaded;
          stage_loaded.stage_ptr = stage_regulator_.stage();
          message_conveyor_(stage_loaded);
          message_dispatcher_(stage_loaded, local_client);
        }

        catch (const std::exception&)
        {
          // TODO: handle error gracefully
        }
      }
    }

    const MessageConveyor& CupEssentials::message_conveyor() const
    {
      return message_conveyor_;
    }

    void CupEssentials::advance_cup()
    {
      cup_controller_.advance();
    }

    void CupEssentials::async_load_stage(stage::StageDescription&& stage_desc)
    {
      stage_loader_.async_load_stage(std::move(stage_desc));
    }

    void CupEssentials::handle_ready_signal(const RemoteClient& client)
    {
      interaction_host_.handle_ready_signal(client);
    }

    void CupEssentials::register_local_client(const LocalConveyor* local_conveyor,
                                              const cup::PlayerDefinition* players, std::size_t player_count)
    {
      message_dispatcher_.initiate_local_connection(local_conveyor);
      interaction_host_.register_client(local_client, players, player_count);
    }
  }
}