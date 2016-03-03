/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "key_settings.hpp"

namespace ts
{
  namespace client
  {
    KeySettings default_key_settings()
    {
      KeySettings key_settings;
      auto& key_mapping = key_settings.key_mapping;

      using controls::Control;    
      using sf::Keyboard;
      key_mapping.define_control(Keyboard::Up, Control::Accelerate, 0);
      key_mapping.define_control(Keyboard::Down, Control::Brake, 0);
      key_mapping.define_control(Keyboard::Left, Control::Left, 0);
      key_mapping.define_control(Keyboard::Right, Control::Right, 0);
      key_mapping.define_control(Keyboard::RControl, Control::Fire, 0);
      key_mapping.define_control(Keyboard::RAlt, Control::AltFire, 0);

      return key_settings;
    }
  }
}