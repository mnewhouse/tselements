/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"
#include "cup_state.hpp"

namespace ts
{
  namespace client
  {
    CupState::CupState(const game_context& context)
      : game::GameState(context),
        client_(context)
    {
    }

    void CupState::process_event(const event_type& event)
    {
      client_.process_event(event);
    }

    void CupState::update(const update_context& update_context)
    {
      client_.update(update_context.frame_duration);
    }
  }
}
