/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CUP_SYNCHRONIZER_HPP_585918891
#define CUP_SYNCHRONIZER_HPP_585918891

#include "cup_messages.hpp"

namespace ts
{
  namespace cup
  {
    class Cup;

    class CupSynchronizer
    {
    public:
      explicit CupSynchronizer(Cup* cup);

      void handle_message(const messages::Intermission& intermission);
      void handle_message(const messages::Initialization& initialization);
      void handle_message(const messages::StageBegin& stage_begin);
      void handle_message(const messages::StageEnd& stage_end);
      void handle_message(const messages::CupEnd& cup_end);
      void handle_message(const messages::Restart& restart);

    private:
      Cup* cup_;
    };
  }
}

#endif