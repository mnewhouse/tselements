/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "entity.hpp"
#include "handling_v2.hpp"

#include "resources/car_definition.hpp"
#include "resources/handling.hpp"

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
    class World;
    using resources::CarDefinition;

    class Car
      : public Entity, public controls::Controllable
    {
    public:
      explicit Car(const CarDefinition& car_definition, std::uint16_t entity_id);
      
      void update(const World& world, double frame_duration);

      const resources::Handling& handling() const { return handling_; }
      const HandlingState& handling_state() const { return handling_state_; }

    private:
      resources::Handling handling_;
      HandlingState handling_state_;
    };
  }
}
