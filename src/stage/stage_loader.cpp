/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"

#include "stage_loader.hpp"
#include "stage.hpp"
#include "stage_creation.hpp"

#include "resources/track_loader.hpp"
#include "resources/pattern_loader.hpp"
#include "resources/pattern_builder.hpp"

namespace ts
{
  namespace stage
  {
    void StageLoader::async_load_stage(stage::StageDescription stage_desc)
    {
      auto loader = [this](stage::StageDescription stage_desc)
      {
        return load_stage(std::move(stage_desc));
      };

      set_loading_state(LoadingState::Initiating);
      GenericLoader::async_load(loader, std::move(stage_desc));
    }

    std::unique_ptr<Stage> StageLoader::load_stage(StageDescription stage_desc)
    {
      set_progress(0.0);
      set_loading_state(LoadingState::LoadingTrack);

      resources::TrackLoader track_loader;
      track_loader.load_from_file(stage_desc.track.path);

      set_loading_state(LoadingState::CreatingEntities);

      auto track = track_loader.get_result();

      auto pattern_loader = preload_pattern_files(track);
      auto pattern = build_track_pattern(track, pattern_loader);

      world::World world_obj(std::move(track), std::move(pattern));

      auto stage_ptr = std::make_unique<stage::Stage>(std::move(world_obj), std::move(stage_desc));

      return stage_ptr;
    }
  }
}