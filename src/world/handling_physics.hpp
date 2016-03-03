/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef HANDLING_PHYSICS_HPP_77721905191355
#define HANDLING_PHYSICS_HPP_77721905191355

namespace ts
{
  namespace resources
  {
    struct TerrainDefinition;
  }

  namespace world
  {
    class Car;

    void update_car_state(Car& car, const resources::TerrainDefinition& terrain, double frame_duration);
  }
}

#endif