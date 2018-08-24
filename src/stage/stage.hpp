/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "stage_description.hpp"
#include "race_tracker.hpp"

#include "world/world.hpp"
#include "world/world_message_fwd.hpp"

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

  namespace controls
  {
    struct ControlsMask;
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

      const StageDescription& stage_description() const;

      const world::World& world() const;
      const resources::Track& track() const;

      void update(std::uint32_t frame_duration, world::EventInterface& event_interface);

      std::uint32_t stage_time() const;

      const RaceTracker* race_tracker() const;      

      void set_controllable_state(std::uint16_t controllable_id, controls::ControlsMask controls_mask);
      void update_car_properties(const world::messages::CarPropertiesUpdate& msg);

      void control_point_hit(const world::Entity* entity, std::uint16_t point_id, std::uint32_t point_flags,
                             std::uint32_t frame_offset, RaceEventInterface& event_interface);

      resources::Track steal_track();

    private:
      void create_stage_entities();

      world::World world_;
      StageDescription stage_description_;
      
      std::uint32_t stage_time_ = 0;
      RaceTracker race_tracker_;
    };
  }
}
