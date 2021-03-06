/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "control_event_translator.hpp"
#include "client_message_dispatcher.hpp"

#include "stage/stage_messages.hpp"
#include "stage/stage.hpp"

#include "controls/key_mapping.hpp"
#include "controls/control_center.hpp"

namespace ts
{
  namespace client
  {
    ControlEventTranslator::ControlEventTranslator(const stage::Stage* stage,
                                                   controls::ControlCenter* control_center,
                                                   KeyMapping key_mapping)
      : stage_(stage),
        control_center_(control_center),
        key_mapping_(std::move(key_mapping))
    {}


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

              message_dispatcher.send(controls_message);
            }
          }
        }
      }

      if (event.type == sf::Event::JoystickMoved)
      {
        std::uint32_t stage_time = stage_->stage_time();

        auto controlled_entities = control_center_->control_slot(0);
        for (auto& controllable : controlled_entities)
        {
          auto v = event.joystickMove.position * 0.01f;

          if (event.joystickMove.axis == sf::Joystick::Z)
          {
            float throttle = std::max(-v, 0.0f);
            float brake = std::max(v, 0.0f);

            controllable.set_control_state(controls::FreeControl::Throttle, throttle);
            controllable.set_control_state(controls::FreeControl::Brake, brake);
          }

          else if (event.joystickMove.axis == sf::Joystick::U)
          {
            float left = std::max(-v, 0.0f);
            float right = std::max(v, 0.0f);

            controllable.set_control_state(controls::FreeControl::Left, left);
            controllable.set_control_state(controls::FreeControl::Right, right);
          }

          stage::messages::ControlUpdate controls_message;
          controls_message.controllable_id = controllable.controllable_id();
          controls_message.controls_mask = controllable.controls_mask();
          controls_message.stage_time = stage_time;

          message_dispatcher.send(controls_message);
        }
      }
    }
  }
}