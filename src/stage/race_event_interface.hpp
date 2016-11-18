/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "stage/race_message_fwd.hpp"

namespace ts
{
  namespace stage
  {
    struct RaceEventInterface
    {
      virtual void on_lap_complete(const messages::LapComplete& lap_event) {}
      virtual void on_sector_complete(const messages::SectorComplete& sector_event) {}
      virtual void on_race_time_update(const messages::RaceTimeUpdate& update) {}
    };
  }
}
