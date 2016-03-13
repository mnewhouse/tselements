/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef WORLD_EVENT_TRANSLATOR_DETAIL_HPP_44219825169
#define WORLD_EVENT_TRANSLATOR_DETAIL_HPP_44219825169

#include "world_event_translator.hpp"
#include "world_messages.hpp"

#include <utility>

namespace ts
{
  namespace world
  {
    template <typename MessageDispatcher>
    EventTranslator<MessageDispatcher>::EventTranslator(MessageDispatcher message_dispatcher)
      : message_dispatcher_(std::move(message_dispatcher))
    {}

    template <typename MessageDispatcher>
    template <typename MessageType>
    void EventTranslator<MessageDispatcher>::dispatch_message(MessageType&& message)
    {
      message_dispatcher_(std::forward<MessageType>(message));
    }

    template <typename MessageDispatcher>
    void EventTranslator<MessageDispatcher>::on_control_point_hit(const Entity* entity, const ControlPoint& point,
                                                                  std::uint32_t frame_offset)
    {
      messages::ControlPointHit message;
      message.entity = entity;
      message.point_id = point.id;
      message.frame_offset = frame_offset;
      dispatch_message(message);      
    }
    
    template <typename MessageDispatcher>
    void EventTranslator<MessageDispatcher>::on_collision(const Entity* entity, const CollisionResult& collision)
    {
      messages::SceneryCollision message;
      message.entity = entity;
      message.collision = collision;
      dispatch_message(message);
    }

    template <typename MessageDispatcher>
    void EventTranslator<MessageDispatcher>::on_collision(const Entity* subject, const Entity* object,
                                                          const CollisionResult& collision)
    {
      messages::EntityCollision message;
      message.subject = subject;
      message.object = object;
      message.collision = collision;
      dispatch_message(message);
    }
  }
}

#endif