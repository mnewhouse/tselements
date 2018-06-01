/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

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
    class MessageDispatcher;

    // The ControlEventTranslator takes game events, translates these events according to the
    // key mapping and the locally controlled entities, and sends them away through the given
    // message dispatcher.
    class ControlEventTranslator
    {
    public:
      ControlEventTranslator(const stage::Stage* stage, controls::ControlCenter* control_center, KeyMapping key_mapping);                             

      void translate_event(const game::Event& event, const MessageDispatcher& message_dispatcher) const;

    private:
      const stage::Stage* stage_;
      controls::ControlCenter* control_center_;      
      KeyMapping key_mapping_;      
    };
  }
}
