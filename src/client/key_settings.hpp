/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "controls/key_mapping.hpp"

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Joystick.hpp>

namespace ts
{
  namespace client
  {
    using KeyMapping = controls::KeyMapping<sf::Keyboard::Key>;

    struct KeySettings
    {
      KeyMapping key_mapping;

      sf::Keyboard::Key zoom_in_key;
      sf::Keyboard::Key zoom_out_key;
    };

    KeySettings default_key_settings();
  }
}
