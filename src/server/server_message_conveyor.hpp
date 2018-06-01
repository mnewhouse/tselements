/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "client_message.hpp"

namespace ts
{
  namespace server
  {   
    class Cup;
    class Stage;

    class MessageConveyor
    {
    public:
      MessageConveyor() = default;

      MessageConveyor(Cup* cup, Stage* stage)
        : cup_(cup), 
          stage_(stage)
      {}

      explicit MessageConveyor(Stage* stage)
        : stage_(stage)
      {
      }

      template <typename MessageType>
      void process(const MessageType& msg) const
      {
        this->process_internal(msg);
      }

    private:
      template <typename MessageType>
      void process_internal(const MessageType& message) const;

      Cup* cup_ = nullptr;
      Stage* stage_ = nullptr;
    };
  }
}