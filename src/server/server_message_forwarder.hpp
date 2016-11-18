/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "client_message.hpp"

#include "cup/cup_message_fwd.hpp"
#include "stage/stage_message_fwd.hpp"
#include "client/update_message.hpp"
#include "world/world_message_fwd.hpp"

namespace ts
{
  namespace server
  {
    class CupEssentials;
    struct MessageForwarder
    {
      CupEssentials* cup_essentials;
      
      void forward(cup::messages::PreInitialization&& pre_initialization);

      void forward(const ClientMessage<cup::messages::Advance>& advance);
      void forward(const ClientMessage<cup::messages::Ready>& ready);
      void forward(const ClientMessage<stage::messages::ControlUpdate>& control_update);
      void forward(const ClientMessage<client::messages::Update>& update);      

      template <typename MessageType>
      void forward(const MessageType& m) {}
    };   
  }
}
