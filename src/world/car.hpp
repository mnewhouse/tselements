/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "entity.hpp"

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
    struct TerrainDefinition;
  }

  namespace world
  {
    using resources::CarDefinition;

    class Car
      : public Entity, public controls::Controllable
    {
    public:
      explicit Car(const CarDefinition& car_definition, EntityId entity_id);

      template <typename TerrainFunc>
      void update(TerrainFunc&& terrain_at, double frame_duration);

      const resources::Handling& handling() const;
      const resources::HandlingState& handling_state() const;

    private:
      resources::Handling handling_;
      resources::HandlingState handling_state_;
    };
  }
}
