/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/


#include "server_message_forwarder.hpp"
#include "server_cup_essentials.hpp"

namespace ts
{
  namespace server
  {
    void MessageForwarder::forward(cup::messages::PreInitialization&& pre_initialization)
    {
      cup_essentials->async_load_stage(std::move(pre_initialization.stage_description));
    }

    void MessageForwarder::forward(const ClientMessage<cup::messages::Advance>& advance)
    {
      cup_essentials->advance_cup();
    }

    void MessageForwarder::forward(const ClientMessage<cup::messages::Ready>& ready)
    {
      cup_essentials->handle_ready_signal(ready.client);
    }

    void MessageForwarder::forward(const ClientMessage<client::messages::Update>& update)
    {
      cup_essentials->update(update.message.frame_duration);
    }

    void MessageForwarder::forward(const ClientMessage<stage::messages::ControlUpdate>& message)
    {
      cup_essentials->forward_stage_message(message);
    }
  }
}