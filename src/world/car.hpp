/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CAR_HPP_812918923
#define CAR_HPP_812918923

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

      void update(const resources::TerrainDefinition& terrain_def, double frame_duration);

      const resources::Handling& handling() const;

      double engine_rev_speed() const;
      double traction() const;
      double load_balance() const;

      const boost::container::small_vector<Vector2i, 4>& tyre_positions() const;

    private:
      double engine_rev_speed_ = 0.0;
      double traction_ = 1.0;
      double load_balance_ = 0.0;
      boost::container::small_vector<Vector2i, 4> tyre_positions_;

      resources::Handling handling_;
    };
  }
}

#endif