/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "server_message_conveyor.hpp"
#include "server_cup_controller.hpp"
#include "server_stage_loader.hpp"
#include "server_interaction_host.hpp"

#include "stage/stage_regulator.hpp"

#include "race/race.hpp"

namespace ts
{
  namespace server
  {
    void forward_message(const MessageContext& context, cup::messages::PreInitialization&& pre_init)
    {
      context.stage_loader->handle_message(std::move(pre_init));
    }

    void forward_message(const MessageContext& context, const world::messages::ControlPointHit& cp_hit)
    {
      if (context.race_host)
      {
        if (auto race = context.race_host->get_ptr())
        {
          race->handle_message(cp_hit);
        }
      }
    }

    void forward_message(const MessageContext& context,
                         const ClientMessage<cup::messages::Advance>& advance)
    {
      context.cup_controller->advance();
    }

    void forward_message(const MessageContext& context, const ClientMessage<cup::messages::Ready>& ready)
    {
      context.interaction_host->handle_ready_signal(ready.client);
    }

    void forward_message(const MessageContext& context,
                         const ClientMessage<stage::messages::ControlUpdate>& control_update)
    {
      context.stage_regulator->handle_message(control_update.message);
    }
  }
}