/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "world/world_message_fwd.hpp"

namespace ts
{
  namespace client
  {
    class ActionState;

    class MessageConveyor
    {
    public:
      explicit MessageConveyor(ActionState* action_state)
        : action_state_(action_state)
      {
      }

      MessageConveyor() = default;

      template <typename MessageType>
      void process(const MessageType& message) const
      {
        this->process_internal(message);
      }

    private:
      template <typename MessageType>
      void process_internal(const MessageType& message) const;

      ActionState* action_state_ = nullptr;
    };
  }
}