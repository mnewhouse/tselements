/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef STAGE_HPP_33311905
#define STAGE_HPP_33311905

#include "stage_description.hpp"

#include "world/world.hpp"

namespace ts
{
  namespace resources
  {
    class Track;
  }

  namespace world
  {
    struct EventInterface;
  }

  namespace stage
  {
    // The Stage class is responsible for tying the game world to the event system.
    // That means that all outside requests to alter the world's state will
    // pass through the interface of this class. A description of the current state
    // is also kept here, for synchronization purposes.
    class Stage
    {
    public:
      explicit Stage(world::World world, StageDescription stage_description);

      void create_stage_entities();
      const StageDescription& stage_description() const;

      const world::World& world() const;
      const resources::Track& track() const;

      void update(world::EventInterface& event_interface, std::uint32_t frame_duration);
      std::uint32_t stage_time() const;

      void set_controllable_state(std::uint16_t controllable_id, std::uint16_t controls_mask);

    private:
      world::World world_;
      StageDescription stage_description_;
      std::uint32_t stage_time_;
    };
  }
}

#endif