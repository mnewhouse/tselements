/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef SERVER_STAGE_ESSENTIALS_DETAIL_HPP_83451897345
#define SERVER_STAGE_ESSENTIALS_DETAIL_HPP_83451897345


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
      template <typename MessageDispatcher, typename MessageConveyor>
      auto make_message_distributor(const MessageDispatcher* dispatcher, const MessageConveyor* conveyor)
      {
        return MessageDistributor<MessageDispatcher, MessageConveyor>(dispatcher, conveyor);
      }

      template <typename MessageDispatcher, typename MessageConveyor>
      auto make_world_event_translator(const MessageDispatcher* dispatcher, const MessageConveyor* conveyor)
      {
        using distributor_type = MessageDistributor<MessageDispatcher, MessageConveyor>;
        return world::EventTranslator<distributor_type>(make_message_distributor(dispatcher, conveyor));
      }

      template <typename MessageDispatcher, typename MessageConveyor>
      struct LapEventTranslator
        : race::LapEventHandler
      {
        using distributor_type = MessageDistributor<MessageDispatcher, MessageConveyor>;

        LapEventTranslator(distributor_type distributor)
          : message_distributor_(distributor)
        {}

        virtual void on_lap_complete(const race::messages::LapComplete& message) override
        {
          message_distributor_(message);
        }

        distributor_type message_distributor_;
      };
    }

    template <typename MessageDispatcher, typename MessageConveyor>
    StageEssentials<MessageDispatcher, MessageConveyor>::StageEssentials(std::unique_ptr<stage::Stage> stage_ptr,
                                                                         const MessageDispatcher* message_dispatcher,
                                                                         const MessageConveyor* message_conveyor)
      : stage_regulator_(std::move(stage_ptr)),
        lap_tracker_(100, stage_regulator_.stage()->track().control_points().size()),
        message_dispatcher_(message_dispatcher),
        message_conveyor_(message_conveyor)
    {}

    template <typename MessageDispatcher, typename MessageConveyor>
    void StageEssentials<MessageDispatcher, MessageConveyor>::update(std::uint32_t frame_duration)
    {
      // Create our own interception interface

      auto event_interface = detail::make_world_event_translator(message_dispatcher_, message_conveyor_);
      stage_regulator_.update(event_interface, frame_duration);

      lap_tracker_.update_race_time(frame_duration);
    }

    template <typename MessageDispatcher, typename MessageConveyor>
    void StageEssentials<MessageDispatcher, MessageConveyor>::handle_message(const ClientMessage<client::messages::Update>& update_message)
    {
      update(update_message.message.frame_duration);
    }

    template <typename MessageDispatcher, typename MessageConveyor>
    void StageEssentials<MessageDispatcher, MessageConveyor>::handle_message(const ClientMessage<stage::messages::ControlUpdate>& update_message)
    {
      stage_regulator_.handle_message(update_message.message);
    }

    template <typename MessageDispatcher, typename MessageConveyor>
    void StageEssentials<MessageDispatcher, MessageConveyor>::handle_message(const world::messages::ControlPointHit& cp_hit)
    {
      using lap_event_translator = detail::LapEventTranslator<MessageDispatcher, MessageConveyor>;

      lap_event_translator translator(detail::make_message_distributor(message_dispatcher_, message_conveyor_));

      lap_tracker_.control_point_hit(cp_hit.entity, cp_hit.point_id, cp_hit.frame_offset, translator);
    }

    template <typename MessageDispatcher, typename MessageConveyor>
    const stage::StageDescription& StageEssentials<MessageDispatcher, MessageConveyor>::stage_description() const
    {
      return stage_regulator_.stage()->stage_description();
    }

    template <typename MessageDispatcher, typename MessageConveyor>
    const stage::Stage* StageEssentials<MessageDispatcher, MessageConveyor>::stage() const
    {
      return stage_regulator_.stage();
    }
  }
}

#endif