/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef SERVER_CUP_CONTROLLER_HPP_2289128359
#define SERVER_CUP_CONTROLLER_HPP_2289128359

#include "server_message_dispatcher.hpp"
#include "server_message_conveyor.hpp"
#include "remote_client_map.hpp"

#include "cup/cup_controller.hpp"

#include <utility>

namespace ts
{
  namespace server
  {
    class MessageConveyor;

    class CupControllerMessageDispatcher
      : private MessageDispatcher
    {
    public:
      explicit CupControllerMessageDispatcher(const MessageDispatcher* message_dispatcher,
                                              const MessageConveyor* message_conveyor);

      template <typename MessageType>
      void operator()(MessageType&& message) const
      {
        (*message_conveyor_)(std::forward<MessageType>(message));

        (*dispatcher_)(std::forward<MessageType>(message), all_clients);
      }

    private:
      const MessageDispatcher* dispatcher_;
      const MessageConveyor* message_conveyor_;
    };

    class CupController
      : public cup::CupController<CupControllerMessageDispatcher>
    {
    public:
      using cup::CupController<CupControllerMessageDispatcher>::CupController;
    };
  }
}

#endif