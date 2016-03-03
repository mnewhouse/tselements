/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CUP_MESSAGE_FWD_HPP_54392859109891
#define CUP_MESSAGE_FWD_HPP_54392859109891

namespace ts
{
  namespace cup
  {
    namespace messages
    {
      struct RegistrationRequest;
      struct RegistrationSuccess;
      struct ServerFull;

      struct Registration;
      struct Intermission;
      struct PreInitialization;
      struct Initialization;
      struct StageBegin;
      struct StageEnd;
      struct CupEnd;
      struct Advance;
      struct Restart;
      struct Ready;

      struct StageBeginRequest;
      struct ClientJoined;      
    }
  }
}

#endif