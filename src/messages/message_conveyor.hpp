/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef MESSAGE_CONVEYOR_HPP_5819818293
#define MESSAGE_CONVEYOR_HPP_5819818293

#include <utility>

namespace ts
{
  namespace messages
  {
    struct DefaultMessageForwarder
    {
      template <typename Context, typename MessageType>
      void operator()(const Context& context, MessageType&& message) const
      {
        this->forward(context, std::forward<MessageType>(message), 0);
      }

      template <typename Context, typename MessageType>
      auto forward(const Context& context, MessageType&& message, int) const
        -> decltype(forward_message(context, std::forward<MessageType>(message)), void())
      {
        // Only enable this overload if there's a matching function, and then
        // we just ADL to forward the message.
        forward_message(context, std::forward<MessageType>(message));
      }

      template <typename Context, typename MessageType>
      void forward(const Context& context, MessageType&& message, ...) const
      {
      }
    };

    // The MessageConveyor class template provides the building blocks to forward 
    // messages to a function as specified in the Forwarder template argument.
    // The forwarder is a callable with a context object as its first parameter and
    // a message as its second parameter.
    template <typename Context, typename Forwarder = DefaultMessageForwarder>
    class MessageConveyor
      : private Forwarder
    {
    public:
      explicit MessageConveyor(Context context, Forwarder forwarder = {})
        : Forwarder(std::move(forwarder)),
          context_(std::move(context))
      {}

      template <typename MessageType>
      void operator()(MessageType&& message) const
      {
        const Forwarder& forwarder = *this;
        forwarder(context_, std::forward<MessageType>(message));
      }

    private:
      Context context_;
    };
  }
}

#endif
