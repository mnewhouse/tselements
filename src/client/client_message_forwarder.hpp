/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "client_action_message_forwarder.hpp"

#include "cup/cup_message_fwd.hpp"
#include "stage/stage_message_fwd.hpp"
#include "stage/race_message_fwd.hpp"

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
      explicit MessageForwarder(CupEssentials<MessageDispatcher>* cup_essentials_)
        : cup_essentials(cup_essentials_)
      {
      }

      void forward(const cup::messages::RegistrationSuccess&);
      void forward(const stage::messages::StageLoaded&);
      void forward(const cup::messages::StageBegin&);
      void forward(const cup::messages::StageEnd&);

      // Have a fallback implementation for unlisted messages that simply does nothing.
      template <typename MessageType>
      void forward(const MessageType& message)
      {
        auto forwarder = action_message_forwarder();

        forwarder.forward(message);
      }

    private:
      ActionMessageForwarder<MessageDispatcher> action_message_forwarder() const;

      CupEssentials<MessageDispatcher>* cup_essentials;
    };
  }
}
