/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "server_stage.hpp"

#include "stage/race_event_translator.hpp"

#include "world/world_event_translator_detail.hpp"
#include "world/world_messages.hpp"

#include "client/client_messages.hpp"

namespace ts
{
  namespace server
  {
    Stage::Stage(std::unique_ptr<stage::Stage> stage_ptr, Cup* cup)
      : stage_regulator_(std::move(stage_ptr)),
      message_dispatcher_(MessageConveyor(cup, this))
    {}

    void Stage::update(std::uint32_t frame_duration)
    {
      // Create our own interception interface
      auto event_interface = world::make_world_event_translator(message_dispatcher_);

      stage_regulator_.update(frame_duration, event_interface);
    }

    void Stage::handle_message(const ClientMessage<client::messages::Update>& update_message)
    {
      update(update_message.message.frame_duration);
    }

    void Stage::handle_message(const ClientMessage<client::messages::LocalConnection>& local_connection)
    {
      message_dispatcher_.connect(local_connection.message.message_conveyor);
    }

    void Stage::handle_message(const ClientMessage<stage::messages::ControlUpdate>& update_message)
    {
      stage_regulator_.handle_message(update_message.message);
    }

    void Stage::handle_message(const world::messages::ControlPointHit& cp_hit)
    {
      auto event_interface = stage::make_race_event_translator(message_dispatcher_);
      stage_regulator_.control_point_hit(cp_hit, event_interface);
    }

    const stage::StageDescription& Stage::stage_description() const
    {
      return stage_regulator_.stage()->stage_description();
    }

    const stage::Stage* Stage::stage() const
    {
      return stage_regulator_.stage();
    }
  }
}
