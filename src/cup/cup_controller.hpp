/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "cup.hpp"
#include "cup_synchronizer.hpp"

#include "cup/cup_messages.hpp"

namespace ts
{
  namespace cup
  {
    class Cup;
    struct CupSettings;

    // The Cup controller class controls the cup, what else.
    // It owns a cup object, and provides a brief interface to act upon it.
    // Cup events are sent through the message dispatcher, which are also internally
    // passed through a cup synchronizer in order to keep things in a consistent state.
    template <typename MessageDispatcher>
    class CupController
    {
    public:
      explicit CupController(const CupSettings& cup_settings, MessageDispatcher dispatcher);      

      void initialize_stage(const stage::StageDescription& stage_desc);
      void advance();
      void restart_cup();

      std::pair<std::uint16_t, RegistrationStatus>
        register_client(const PlayerDefinition* players, std::size_t player_count);

      void unregister_client(std::uint16_t client_id);
      void handle_ready_signal(std::uint16_t client_id);

      const Cup& cup() const;

    private:
      template <typename MessageType>
      void dispatch_message(MessageType&& message);

      bool is_everyone_ready() const;

      void begin_cup();
      void intermission();
      void preinitialize_stage();
      void begin_stage();
      void end_stage();
      void end_cup();

      std::vector<std::uint8_t> client_ready_states_;
      
      Cup cup_;
      CupSynchronizer cup_synchronizer_;
      MessageDispatcher message_dispatcher_;
    };
  }
}
