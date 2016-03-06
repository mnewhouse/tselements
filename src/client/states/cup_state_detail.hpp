/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CUP_STATE_DETAIL_HPP_338129812893
#define CUP_STATE_DETAIL_HPP_338129812893

#include "cup_state.hpp"

namespace ts
{
  namespace client
  {
    template <typename MessageDispatcher>
    CupState<MessageDispatcher>::CupState(const game_context& context)
      : game::GameState(context),
        client_(context)
    {
    }

    template <typename MessageDispatcher>
    void CupState<MessageDispatcher>::process_event(const event_type& event)
    {
      client_.process_event(event);
    }
    
    template <typename MessageDispatcher>
    void CupState<MessageDispatcher>::update(const update_context& update_context)
    {
      client_.update(update_context.frame_duration);
    }
  }
}

#endif