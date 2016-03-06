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

    // The CupSynchronizer takes various cup messages, and updates a cup object based on
    // these events. This way, this same mechanism can be used for both incoming and outgoing messages,
    // i.e. both client and server.
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
      void handle_message(const messages::PreInitialization& pre_initialization);

    private:
      Cup* cup_;
    };
  }
}

#endif