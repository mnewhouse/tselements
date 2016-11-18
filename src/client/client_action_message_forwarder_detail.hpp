/* 
* TS Elements
* Copyright 2015 - 2016 M.Newhouse
* Released under the MIT license.
*/

#pragma once

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
