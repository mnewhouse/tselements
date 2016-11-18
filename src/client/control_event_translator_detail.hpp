/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "control_event_translator.hpp"

namespace ts
{
  namespace client
  {
    template <typename MessageDispatcher>
    void ControlEventTranslator::translate_event(const game::Event& event, const MessageDispatcher& message_dispatcher) const
    {
      if (event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased)
      {
        bool key_state = (event.type == sf::Event::KeyPressed);
        std::uint32_t stage_time = stage_->stage_time();

        // Translate the keystroke to a control/slot combination
        auto mapped_keys = key_mapping_.controls_by_key(event.key.code);
        for (auto entry : mapped_keys)
        {
          // If the slot is currently being used, dispatch a control message to the server
          // with the slot's controllable id(s) and the control state.
          auto controlled_entities = control_center_->control_slot(entry.slot);
          for (auto& controllable : controlled_entities)
          {
            // We also need to test if the control state has actually changed.
            if (controllable.set_control_state(entry.control, key_state))
            {
              stage::messages::ControlUpdate controls_message;
              controls_message.controllable_id = controllable.controllable_id();
              controls_message.controls_mask = controllable.controls_mask();
              controls_message.stage_time = stage_time;

              message_dispatcher(controls_message);
            }
          }
        }
      }
    }
  }
}
