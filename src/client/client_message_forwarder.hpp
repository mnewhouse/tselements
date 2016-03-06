/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CLIENT_MESSAGE_FORWARDER_HPP_242418581915
#define CLIENT_MESSAGE_FORWARDER_HPP_242418581915

#include "cup/cup_message_fwd.hpp"
#include "stage/stage_message_fwd.hpp"

namespace ts
{
  namespace client
  {
    template <typename MessageDispatcher>
    class CupEssentials;

    // This class template defines a fixed set of message forwarding overloads to pass
    // to the essential client cup logic.
    template <typename MessageDispatcher>
    class MessageForwarder
    {
    public:
      using cup_essentials = CupEssentials<MessageDispatcher>;

      void forward(cup_essentials&, const cup::messages::RegistrationSuccess&);
      void forward(cup_essentials&, const stage::messages::StageLoaded&);
      void forward(cup_essentials&, const cup::messages::StageBegin&);
      void forward(cup_essentials&, const cup::messages::StageEnd&);

      // Have a fallback implementation for unlisted messages that simply does nothing.
      template <typename MessageType>
      void forward(cup_essentials&, const MessageType&) {}
    };
  }
}

#endif