/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"
#include "cup_synchronizer.hpp"
#include "cup.hpp"

namespace ts
{
  namespace cup
  {
    CupSynchronizer::CupSynchronizer(Cup* cup)
      : cup_(cup)
    {
    }

    void CupSynchronizer::handle_message(const messages::Intermission& intermission)
    {
      cup_->set_cup_state(CupState::Intermission);
      cup_->set_current_stage(intermission.stage_id);
    }

    void CupSynchronizer::handle_message(const messages::Initialization& initialization)
    {
      cup_->set_cup_state(CupState::Initialization);
    }

    void CupSynchronizer::handle_message(const messages::PreInitialization& pre_initialization)
    {
      cup_->set_cup_state(CupState::PreInitialization);
    }

    void CupSynchronizer::handle_message(const messages::StageBegin& stage_begin)
    {
      cup_->set_cup_state(CupState::Action);
    }

    void CupSynchronizer::handle_message(const messages::StageEnd& stage_end)
    {
      cup_->set_current_stage(stage_end.stage_id + 1);
    }

    void CupSynchronizer::handle_message(const messages::CupEnd& cup_end)
    {
      cup_->set_cup_state(CupState::End);
    }

    void CupSynchronizer::handle_message(const messages::Restart& restart)
    {
      cup_->restart();
    }
  }
}