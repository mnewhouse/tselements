/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CONTROL_EVENT_TRANSLATOR_HPP_671834671
#define CONTROL_EVENT_TRANSLATOR_HPP_671834671

#include "client/key_settings.hpp"

#include "game/game_events.hpp"

namespace ts
{
  namespace stage
  {
    class Stage;
  }

  namespace controls
  {
    class ControlCenter;
  }

  namespace client
  {
    class ControlEventTranslator
    {
    public:
      ControlEventTranslator(const stage::Stage* stage, controls::ControlCenter* control_center, KeyMapping key_mapping);                             

      template <typename MessageDispatcher>
      void translate_event(const game::Event& event, const MessageDispatcher& message_dispatcher) const;

    private:
      const stage::Stage* stage_;
      controls::ControlCenter* control_center_;      
      KeyMapping key_mapping_;      
    };
  }
}

#endif