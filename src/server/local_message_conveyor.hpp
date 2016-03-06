/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef LOCAL_MESSAGE_CONVEYOR_HPP_4343019185
#define LOCAL_MESSAGE_CONVEYOR_HPP_4343019185

namespace ts
{
  namespace client
  {
    class LocalMessageDispatcher;

    template <typename MessageConveyor>
    class MessageConveyor;
  }

  namespace server
  {
    using LocalConveyor = client::MessageConveyor<client::LocalMessageDispatcher>;
  }
}

#endif