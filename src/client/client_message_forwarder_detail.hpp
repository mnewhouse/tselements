/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "client_message_forwarder.hpp"
#include "client_cup_essentials.hpp"
#include "client_action_message_forwarder_detail.hpp"

#include "cup/cup_messages.hpp"
#include "stage/stage_messages.hpp"

namespace ts
{
  namespace client
  {
    template <typename MessageDispatcher>
    void MessageForwarder<MessageDispatcher>::forward(const cup::messages::RegistrationSuccess& message)
    {
      cup_essentials->registration_success(message.client_id, message.client_key);
    }

    template <typename MessageDispatcher>
    void MessageForwarder<MessageDispatcher>::forward(const stage::messages::StageLoaded& message)
    {
      cup_essentials->async_load_scene(message.stage_ptr);
    }

    template <typename MessageDispatcher>
    void MessageForwarder<MessageDispatcher>::forward(const cup::messages::StageBegin&)
    {
      cup_essentials->launch_action();
    }

    template <typename MessageDispatcher>
    void MessageForwarder<MessageDispatcher>::forward(const cup::messages::StageEnd&)
    {
      cup_essentials->end_action();
    }

    template <typename MessageDispatcher>
    ActionMessageForwarder<MessageDispatcher> MessageForwarder<MessageDispatcher>::action_message_forwarder() const
    {
      return{ cup_essentials->action_essentials() };
    }
  }
}
