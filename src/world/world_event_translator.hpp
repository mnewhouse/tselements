/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

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
      virtual void on_collision(const Entity* subject, const Entity* object,
                                const CollisionResult& collision) override;

    private:
      template <typename MessageType>
      void dispatch_message(MessageType&& message);

      MessageDispatcher message_dispatcher_;
    };

    template <typename MessageDispatcher>
    auto make_world_event_translator(MessageDispatcher dispatcher)
    {
      return world::EventTranslator<MessageDispatcher>(dispatcher);
    }
  }
}
