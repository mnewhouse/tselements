/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef STAGE_CREATION_HPP_6611289
#define STAGE_CREATION_HPP_6611289

namespace ts
{
  namespace world
  {
    class World;
  }

  namespace stage
  {
    struct StageDescription;

    void create_stage_entities(world::World& world, const StageDescription& stage_description);
  }
}

#endif