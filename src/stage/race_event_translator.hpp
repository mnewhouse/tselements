/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
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
        message_dispatcher_.send(lap_event);
      }

      virtual void on_sector_complete(const messages::SectorComplete& sector_event) override
      {
        message_dispatcher_.send(sector_event);
      }

      virtual void on_race_time_update(const messages::RaceTimeUpdate& update) override
      {
        message_dispatcher_.send(update);
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
