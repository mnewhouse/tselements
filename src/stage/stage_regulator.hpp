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

      void update(world::EventInterface& event_interface, std::uint32_t frame_duration);
      bool active() const;

      template <typename MessageType>
      void handle_message(const MessageType& message);

    private:
      template <typename MessageType>
      void handle_message(const MessageType& message, void*);

      template <typename MessageType>
      auto handle_message(const MessageType& message, int) -> decltype(handle_message_(message), void());

      void handle_message_(const messages::ControlUpdate& control_message);

      std::unique_ptr<Stage> stage_;
    };

    template <typename MessageType>
    void StageRegulator::handle_message(const MessageType& message)
    {
      handle_message(message, 0);
    }

    template <typename MessageType>
    void StageRegulator::handle_message(const MessageType& message, void*)
    {
    }

    template <typename MessageType>
    auto StageRegulator::handle_message(const MessageType& message, int) 
      -> decltype(handle_message_(message), void())
    {
      if (stage_)
      {
        handle_message_(message);
      }
    }
  }
}

#endif