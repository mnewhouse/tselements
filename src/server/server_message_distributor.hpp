/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef SERVER_MESSAGE_DISTRIBUTOR_HPP_49308950893
#define SERVER_MESSAGE_DISTRIBUTOR_HPP_49308950893

#include <utility>

#include "server_message_dispatcher.hpp"
#include "server_message_conveyor.hpp"

namespace ts
{
  namespace server
  {
    template <typename MessageDispatcher, typename MessageConveyor>
    class MessageDistributor
    {
    public:
      explicit MessageDistributor(const MessageDispatcher* dispatcher,
                                  const MessageConveyor* conveyor)
        : dispatcher_(dispatcher),
          conveyor_(conveyor)
      {}

      template <typename MessageType>
      void operator()(MessageType&& message) const
      {
        (*conveyor_)(std::forward<MessageType>(message));

        (*dispatcher_)(std::forward<MessageType>(message), all_clients);
      }

    private:
      const MessageDispatcher* dispatcher_;
      const MessageConveyor* conveyor_;
    };

    using DefaultMessageDistributor = MessageDistributor<MessageDispatcher, MessageConveyor>;
  }
}


#endif