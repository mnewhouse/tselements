/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/


#include "stage_creation.hpp"
#include "stage_description.hpp"

#include "world/world.hpp"

namespace ts
{
  namespace stage
  {
    void create_stage_entities(world::World& world, const StageDescription& stage_desc)
    {
      for (const auto& car : stage_desc.car_instances)
      {
        world.create_car(stage_desc.car_models[car.model_id], car.instance_id, car.start_pos);
      }
    }
  }
}