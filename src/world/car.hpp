/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "entity.hpp"
#include "handling_v2.hpp"

#include "resources/car_definition.hpp"
#include "resources/handling_properties.hpp"

#include "controls/controllable.hpp"

#include <boost/container/small_vector.hpp>

#include <cstdint>
#include <vector>

namespace ts
{
  namespace resources
  {
    struct CarDefinition;
  }

  namespace world
  {
    class TerrainMap;
    using resources::CarDefinition;

    class Car
      : public Entity, public controls::Controllable
    {
    public:
      explicit Car(const CarDefinition& car_definition, std::uint16_t entity_id);
      
      void update(const TerrainMap& terrain_map, double frame_duration);

      const resources::HandlingProperties& handling_properties() const { return handling_properties_; }
      const HandlingState& handling_state() const { return handling_state_; }

    private:
      resources::HandlingProperties handling_properties_;
      HandlingState handling_state_;
    };
  }
}
