/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "server.hpp"
#include "server_message_conveyor.hpp"
#include "server_message_dispatcher.hpp"
#include "server_interaction_host.hpp"
#include "server_cup_controller.hpp"
#include "server_stage_loader.hpp"
#include "remote_client_map.hpp"

#include "cup/cup.hpp"
#include "cup/cup_settings.hpp"
#include "cup/cup_messages.hpp"

#include "resources/resource_store.hpp"
#include "resources/settings.hpp"

#include "stage/stage_regulator.hpp"
#include "stage/stage.hpp"
#include "stage/stage_messages.hpp"

#include "world/world_event_translator.hpp"
#include "world/world_event_translator_detail.hpp"

#include "race/race.hpp"

#include <boost/optional.hpp>

namespace ts
{
  namespace server
  {
    namespace detail
    {
      struct ConveyingMessageDispatcher
      {
        ConveyingMessageDispatcher(const MessageConveyor* conveyor, const MessageDispatcher* dispatcher)
          : conveyor_(conveyor),
            dispatcher_(dispatcher)
        {
        }

        template <typename MessageType>
        void operator()(const MessageType& message) const
        {
          (*conveyor_)(message);
          (*dispatcher_)(message);
        }

      private:
        const MessageConveyor* conveyor_;
        const MessageDispatcher* dispatcher_;
      };
    }


    struct Server::Impl
    {
      explicit Impl(resources::ResourceStore* resource_store);

      resources::ResourceStore* resource_store_;

      stage::StageRegulator stage_regulator_;
      StageLoader stage_loader_;

      CupController cup_controller_;
      InteractionHost interaction_host_;

      MessageConveyor message_conveyor_;
      MessageDispatcher message_dispatcher_;

      world::EventTranslator<detail::ConveyingMessageDispatcher> world_event_translator_;

      race::RaceHost race_;

      MessageContext make_message_context();
    };

    Server::Impl::Impl(resources::ResourceStore* resource_store)
      : resource_store_(resource_store),
        stage_regulator_(),
        stage_loader_(),
        cup_controller_(resource_store->settings().cup_settings(), 
                        CupControllerMessageDispatcher(&message_dispatcher_, &message_conveyor_)),
        interaction_host_(&cup_controller_, &message_dispatcher_),
        message_conveyor_(make_message_context()),
        message_dispatcher_(),
        world_event_translator_(detail::ConveyingMessageDispatcher(&message_conveyor_, &message_dispatcher_))
    {}

    MessageContext Server::Impl::make_message_context()
    {
      MessageContext context;
      context.cup_controller = &cup_controller_;
      context.stage_regulator = &stage_regulator_;
      context.stage_loader = &stage_loader_;
      context.interaction_host = &interaction_host_;
      context.race_host = &race_;
      return context;
    }

    Server::Server(resources::ResourceStore* resource_store)
      : impl_(std::make_unique<Impl>(resource_store))
    {
    }

    Server::~Server()
    {
    }

    void Server::update(std::uint32_t frame_duration)
    {
      if (impl_->stage_regulator_.active())
      {
        impl_->stage_regulator_.update(impl_->world_event_translator_, frame_duration);
      }

      if (impl_->race_)
      {
        impl_->race_->update_race_time(frame_duration);
      }

      if (impl_->stage_loader_.is_ready())
      {
        impl_->stage_regulator_.adopt_stage(impl_->stage_loader_.get_result());

        const auto& stage_desc = impl_->stage_regulator_.stage()->stage_description();
        impl_->cup_controller_.initialize_stage(stage_desc);

        stage::messages::StageLoaded stage_loaded;
        stage_loaded.stage_ptr = impl_->stage_regulator_.stage();
        impl_->message_conveyor_(stage_loaded);
        impl_->message_dispatcher_(stage_loaded, local_client);

        impl_->race_.emplace(10, stage()->track().control_points().size());
      }
    }

    const stage::Stage* Server::stage() const
    {
      return impl_->stage_regulator_.stage();
    }

    void Server::initiate_local_connection(const client::MessageConveyor* message_conveyor,
                                           const cup::PlayerDefinition* players, std::size_t player_count)
    {
      impl_->message_dispatcher_.initiate_local_connection(message_conveyor);

      impl_->interaction_host_.register_client(local_client, players, player_count);
    }

    const MessageConveyor& Server::message_conveyor() const
    {
      return impl_->message_conveyor_;
    }

    const cup::Cup& Server::cup() const
    {
      return impl_->cup_controller_.cup();
    }
  }
}