/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "server_stage_essentials.hpp"
#include "server_message_distributor.hpp"

#include "world/world_event_translator_detail.hpp"
#include "world/world_messages.hpp"

namespace ts
{
  namespace server
  {
    namespace detail
    {
      auto make_message_distributor(const MessageDispatcher* dispatcher, const MessageConveyor* conveyor)
      {
        return MessageDistributor(dispatcher, conveyor);
      }

      auto make_world_event_translator(const MessageDispatcher* dispatcher, const MessageConveyor* conveyor)
      {
        return world::EventTranslator<MessageDistributor>(make_message_distributor(dispatcher, conveyor));
      }

      struct LapEventTranslator
        : race::LapEventHandler
      {
        LapEventTranslator(MessageDistributor distributor)
          : message_distributor_(distributor)
        {}

        virtual void on_lap_complete(const race::messages::LapComplete& message) override
        {
          message_distributor_(message);
        }

        MessageDistributor message_distributor_;
      };
    }

    StageEssentials::StageEssentials(std::unique_ptr<stage::Stage> stage_ptr,
                                     const MessageDispatcher* message_dispatcher,
                                     const MessageConveyor* message_conveyor)
      : stage_regulator_(std::move(stage_ptr)),
        lap_tracker_(10, stage_regulator_.stage()->track().control_points().size()),
        message_dispatcher_(message_dispatcher),
        message_conveyor_(message_conveyor)
    {}

    void StageEssentials::update(std::uint32_t frame_duration)
    {
      // Create our own interception interface

      auto event_interface = detail::make_world_event_translator(message_dispatcher_, message_conveyor_);
      stage_regulator_.update(event_interface, frame_duration);
    }

    void StageEssentials::handle_message(const ClientMessage<stage::messages::ControlUpdate>& update_message)
    {
      stage_regulator_.handle_message(update_message.message);
    }

    void StageEssentials::handle_message(const world::messages::ControlPointHit& cp_hit)
    {
      detail::LapEventTranslator translator(detail::make_message_distributor(message_dispatcher_, 
                                                                             message_conveyor_));

      lap_tracker_.control_point_hit(cp_hit.entity, cp_hit.point_id, cp_hit.frame_offset, translator);
    }

    const stage::StageDescription& StageEssentials::stage_description() const
    {
      return stage_regulator_.stage()->stage_description();
    }

    const stage::Stage* StageEssentials::stage() const
    {
      return stage_regulator_.stage();
    }
  }
}