/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "utility/generic_loader.hpp"

#include "stage_description.hpp"

namespace ts
{
  namespace stage
  {
    class Stage;

    enum class LoadingState
    {
      Initiating,
      LoadingTrack,
      BuildingPattern,
      CreatingEntities,
      Finished
    };

    class StageLoader
      : public utility::GenericLoader<LoadingState, std::unique_ptr<Stage>>
    {
    public:
      std::unique_ptr<Stage> load_stage(StageDescription stage_desc);

      void async_load_stage(StageDescription stage_desc);
    };
  }
}
