/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef STAGE_REGULATOR_HPP_41829358
#define STAGE_REGULATOR_HPP_41829358

#include "stage.hpp"
#include "stage_messages.hpp"
#include "stage_loader.hpp"

#include <memory>

namespace ts
{
  namespace stage
  {
    // The stage regulator class is responsible for translating any stage-related events
    // into actual things that happen in the game world.
    class StageRegulator
    {
    public:
      void adopt_stage(std::unique_ptr<Stage> stage_ptr);
      void destroy_stage();

      const Stage* stage() const;
      explicit operator bool() const;

      void update(world::EventInterface& event_interface, std::uint32_t frame_duration);
      bool active() const;

      void handle_message(const messages::ControlUpdate& control_message);

      std::unique_ptr<Stage> stage_;
    };
  }
}

#endif