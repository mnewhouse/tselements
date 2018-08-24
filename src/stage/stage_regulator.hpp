/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "stage.hpp"
#include "stage_messages.hpp"
#include "stage_loader.hpp"

#include "world/world_message_fwd.hpp"

#include <memory>

namespace ts
{
  namespace stage
  {
    struct RaceEventInterface;

    // The stage regulator class is responsible for translating any stage-related events
    // into actual things that happen in the game world.
    class StageRegulator
    {
    public:
      explicit StageRegulator(std::unique_ptr<Stage> stage_ptr);

      const Stage* stage() const;

      void update(std::uint32_t frame_duration, world::EventInterface& event_interface);

      void handle_message(const messages::ControlUpdate& control_message);
      void handle_message(const world::messages::CarPropertiesUpdate& car_update);

      void control_point_hit(const world::messages::ControlPointHit& cp_hit,
                             RaceEventInterface& event_interface);

    private:
      std::unique_ptr<Stage> stage_;
    };
  }
}
