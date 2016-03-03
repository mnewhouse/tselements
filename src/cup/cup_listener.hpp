/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CUP_LISTENER_HPP_3319258129
#define CUP_LISTENER_HPP_3319258129

#include "cup_messages.hpp"

namespace ts
{
  namespace cup
  {
    struct CupListener
    {      
      virtual ~CupListener() = default;

      virtual void handle_message(const messages::Intermission&) {}
      virtual void handle_message(const messages::Initialization&) {}
      virtual void handle_message(const messages::StageBegin&) {}
      virtual void handle_message(const messages::StageEnd&) {}
      virtual void handle_message(const messages::CupEnd&) {}
      virtual void handle_message(const messages::Advance&) {}
      virtual void handle_message(const messages::Restart&) {}
    };
  }
}

#endif