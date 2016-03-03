/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CUP_SETTINGS_HPP_668596182391
#define CUP_SETTINGS_HPP_668596182391

#include "car_mode.hpp"

#include "resources/track_reference.hpp"
#include "resources/car_definition.hpp"

#include <vector>

namespace ts
{
  namespace cup
  {
    struct CupSettings
    {
      std::vector<resources::TrackReference> tracks;
      std::vector<resources::CarDefinition> selected_cars;
      CarMode car_mode = CarMode::Free;
      std::size_t max_players = 20;
    };
  }
}

#endif