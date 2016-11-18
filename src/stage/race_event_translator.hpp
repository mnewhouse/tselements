/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "race_event_interface.hpp"

namespace ts
{
  namespace stage
  {
    template <typename MessageDispatcher>
    struct RaceEventTranslator
      : RaceEventInterface
    {
      explicit RaceEventTranslator(MessageDispatcher md)
        : message_dispatcher_(md)
      {}

      virtual void on_lap_complete(const messages::LapComplete& lap_event) override
      {
        message_dispatcher_(lap_event);
      }

      virtual void on_sector_complete(const messages::SectorComplete& sector_event) override
      {
        message_dispatcher_(sector_event);
      }

      virtual void on_race_time_update(const messages::RaceTimeUpdate& update) override
      {
        message_dispatcher_(update);
      }

      MessageDispatcher message_dispatcher_;
    };

    template <typename MessageDispatcher>
    auto make_race_event_translator(MessageDispatcher dispatcher)
    {
      return RaceEventTranslator<MessageDispatcher>(dispatcher);
    }
  }
}
