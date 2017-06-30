/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/


#include "server_cup_essentials.hpp"
#include "server_stage_essentials.hpp"
#include "server_message_distributor.hpp"

#include "resources/resource_store.hpp"
#include "resources/settings.hpp"

#include <iostream>

namespace ts
{
  namespace server
  {
    CupEssentials::CupEssentials(resources::ResourceStore* resource_store)
      : resource_store_(resource_store),
      message_conveyor_(MessageContext{ this }),
      message_dispatcher_(),
      cup_controller_(resource_store->settings().cup_settings(),
                      DefaultMessageDistributor(&message_dispatcher_, &message_conveyor_)),
      interaction_host_(&cup_controller_, &message_dispatcher_),
      stage_loader_(),
      stage_essentials_()
    {}

    void CupEssentials::update(std::uint32_t frame_duration)
    {
      if (stage_essentials_)
      {        
        stage_essentials_->update(frame_duration);
      }

      if (stage_loader_.is_ready())
      {
        initialize_loaded_stage();
      }
    }

    void CupEssentials::initialize_loaded_stage()
    {
      try
      {
        // Get the stage from the stage loader, and initialize the stage essentials object with it.
        stage_essentials_.emplace(stage_loader_.get_result(), &message_dispatcher_, &message_conveyor_);
        const auto& stage_desc = stage_essentials_->stage_description();

        cup_controller_.initialize_stage(stage_desc);

        // Now inform ourselves and possibly the local client that we loaded the stage.
        stage::messages::StageLoaded stage_loaded;
        stage_loaded.stage_ptr = stage_essentials_->stage();

        message_conveyor_(stage_loaded);
        message_dispatcher_(stage_loaded, local_client);        
      }

      catch (const std::exception& e)
      {
        // TODO: handle error gracefully
        std::cout << e.what() << std::endl;
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