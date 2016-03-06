/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef WORLD_EVENT_TRANSLATOR_HPP_777891281833
#define WORLD_EVENT_TRANSLATOR_HPP_777891281833

#include "world_event_interface.hpp"

#include <type_traits>

namespace ts
{
  namespace world
  {
    template <typename MessageDispatcher>
    class EventTranslator
      : public EventInterface
    {
    public:
      explicit EventTranslator(MessageDispatcher message_dispatcher);

      virtual void on_control_point_hit(const Entity* entity, const ControlPoint& point,
                                        std::uint32_t frame_offset) override;

      virtual void on_collision(const Entity* entity, const CollisionResult& collision) override;

    private:
      template <typename MessageType>
      void dispatch_message(MessageType&& message);

      MessageDispatcher message_dispatcher_;
    };
  }
}

#endif