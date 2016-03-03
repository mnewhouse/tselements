/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef ACTION_STATE_HPP_66892213
#define ACTION_STATE_HPP_66892213

#include "game/game_state.hpp"

#include <memory>

namespace ts
{
  namespace scene
  {
    struct Scene;
  }

  namespace controls
  {
    class ControlCenter;
  }

  namespace client
  {
    class ControlEventTranslator;
    
    struct UpdateInterface
    {
    public:
      explicit UpdateInterface(game::GameState* parent_state)
        : parent_state_(parent_state)
      {
      }

      void update(const game::UpdateContext& update_context) const
      {
        parent_state_->update(update_context);
      }

    private:
      game::GameState* parent_state_;
    };


    template <typename MessageDispatcher>
    class ActionState
      : public game::GameState
    {
    public:
      ActionState(const game_context& game_context, scene::Scene scene, 
                  controls::ControlCenter control_center, const MessageDispatcher* message_dispatcher,
                  UpdateInterface update_interface);

      virtual void render(const render_context&) const override;
      virtual void update(const update_context&) override;
      virtual void process_event(const event_type&) override;

    private:
      struct Impl;
      std::unique_ptr<Impl> impl_;
    };
  }
}

#endif