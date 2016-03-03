/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef SERVER_MESSAGE_CONVEYOR_HPP_2389418235
#define SERVER_MESSAGE_CONVEYOR_HPP_2389418235

#include "remote_client.hpp"
#include "client_message.hpp"

#include "messages/message_conveyor.hpp"

#include "cup/cup_message_fwd.hpp"

#include "world/world_messages.hpp"

namespace ts
{
  namespace stage
  {
    class StageRegulator;

    namespace messages
    {
      struct ControlUpdate;
    }
  }

  namespace race
  {
    struct RaceHost;
  }

  namespace server
  {
    class CupController;
    class StageLoader;
    class InteractionHost;

    struct MessageContext
    {
      stage::StageRegulator* stage_regulator = nullptr;
      InteractionHost* interaction_host = nullptr;
      CupController* cup_controller = nullptr;
      StageLoader* stage_loader = nullptr;
      race::RaceHost* race_host = nullptr;
    };

    class MessageConveyor
      : public messages::MessageConveyor<MessageContext>
    {
      using messages::MessageConveyor<MessageContext>::MessageConveyor;
    };

    // The following overloads are internal messages that we use to communicate between different
    // server components. They are not sent to the clients.
    void forward_message(const MessageContext& context, cup::messages::PreInitialization&& pre_init);

    void forward_message(const MessageContext& context, const world::messages::ControlPointHit& cp_hit);

    // The following overloads are all the message types that are supported as incoming client messages.
    // Passing any other message types to the MessageConveyor will result in a compilation error.

    // Control message
    void forward_message(const MessageContext& context, 
                         const ClientMessage<stage::messages::ControlUpdate>& control_update);

    void forward_message(const MessageContext& context,
                         const ClientMessage<cup::messages::Advance>& advance);

    void forward_message(const MessageContext& context, 
                         const ClientMessage<cup::messages::Ready>& ready);

    // TODO: Connection request, quit message, chat message, ready-for-action message.    
  }
}

#endif