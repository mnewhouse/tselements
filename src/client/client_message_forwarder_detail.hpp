/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CLIENT_MESSAGE_FORWARDER_DETAIL_HPP_8198125258583
#define CLIENT_MESSAGE_FORWARDER_DETAIL_HPP_8198125258583

#include "client_message_forwarder.hpp"
#include "client_cup_essentials.hpp"

#include "cup/cup_messages.hpp"

#include "stage/stage_messages.hpp"

namespace ts
{
  namespace client
  {
    template <typename MessageDispatcher>
    void MessageForwarder<MessageDispatcher>::forward(cup_essentials& essentials,
                                                      const cup::messages::RegistrationSuccess& message)
    {
      essentials.registration_success(message.client_id, message.client_key);
    }

    template <typename MessageDispatcher>
    void MessageForwarder<MessageDispatcher>::forward(cup_essentials& essentials,
                                                      const stage::messages::StageLoaded& message)
    {
      essentials.async_load_scene(message.stage_ptr);
    }

    template <typename MessageDispatcher>
    void MessageForwarder<MessageDispatcher>::forward(cup_essentials& essentials, 
                                                      const cup::messages::StageBegin&)
    {
      essentials.launch_action();
    }

    template <typename MessageDispatcher>
    void MessageForwarder<MessageDispatcher>::forward(cup_essentials& essentials, 
                                                      const cup::messages::StageEnd&)
    {
      essentials.end_action();
    }
  }
}

#endif