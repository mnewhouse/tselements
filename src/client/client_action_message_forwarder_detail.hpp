/* 
* TS Elements
* Copyright 2015 - 2016 M.Newhouse
* Released under the MIT license.
*/

#ifndef CLIENT_ACTION_MESSAGE_FORWARDER_DETAIL_HPP_845691238746
#define CLIENT_ACTION_MESSAGE_FORWARDER_DETAIL_HPP_845691238746

#include "client_action_essentials.hpp"

namespace ts
{
  namespace client
  {
    template <typename MessageDispatcher>
    void ActionMessageForwarder<MessageDispatcher>::forward(const world::messages::SceneryCollision& collision)
    {
      action_essentials->collision_event(collision);
    }

    template <typename MessageDispatcher>
    void ActionMessageForwarder<MessageDispatcher>::forward(const world::messages::EntityCollision& collision)
    {
      action_essentials->collision_event(collision);
    }
  }
}

#endif