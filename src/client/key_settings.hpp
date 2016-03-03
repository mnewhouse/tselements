/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CLIENT_CONTROLS_HPP_48192378
#define CLIENT_CONTROLS_HPP_48192378

#include "controls/key_mapping.hpp"

#include <SFML/Window/Keyboard.hpp>

namespace ts
{
  namespace client
  {
    using KeyMapping = controls::KeyMapping<sf::Keyboard::Key>;

    struct KeySettings
    {
      KeyMapping key_mapping;
    };

    KeySettings default_key_settings();
  }
}

#endif