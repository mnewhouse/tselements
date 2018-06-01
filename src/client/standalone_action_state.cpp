/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"
#include "standalone_action_state.hpp"

namespace ts
{
  namespace client
  {
    StandaloneActionState::StandaloneActionState(game::GameContext game_context,
                                                 scene::Scene scene_obj,
                                                 const LocalPlayerRoster& local_players,
                                                 std::unique_ptr<stage::Stage> stage_ptr)
      : ActionState(game_context, std::move(scene_obj), local_players),
        server_stage_(std::move(stage_ptr))
    {
      connect(server::MessageConveyor(&server_stage_));
    }
  }
}