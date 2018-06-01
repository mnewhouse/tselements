/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "action_state.hpp"

#include "server/server_stage.hpp"

namespace ts
{
  namespace client
  {
    class StandaloneActionState
      : public ActionState
    {
    public:
      StandaloneActionState(game::GameContext game_context, scene::Scene scene_obj, const LocalPlayerRoster& local_player,
                            std::unique_ptr<stage::Stage> stage_ptr);


    private:
      server::Stage server_stage_;
    };
  }
}