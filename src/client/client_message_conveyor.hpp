/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CLIENT_MESSAGE_CONVEYOR_HPP_668912385
#define CLIENT_MESSAGE_CONVEYOR_HPP_668912385

#include "messages/message_conveyor.hpp"

#include "cup/cup_message_fwd.hpp"

namespace ts
{
  namespace cup
  {
    class CupSynchronizer;
  }

  namespace stage
  {
    namespace messages
    {
      struct StageLoaded;
    }
  }

  namespace client
  {
    struct SceneLoaderInterface;
    struct CupStateInterface;

    class LocalPlayerController;
    

    struct MessageContext
    {
      CupStateInterface* cup_state_interface = nullptr;
      cup::CupSynchronizer* cup_synchronizer = nullptr;
      SceneLoaderInterface* scene_loader = nullptr;
      LocalPlayerController* local_player_controller = nullptr;
    };

    class MessageConveyor
      : public messages::MessageConveyor<MessageContext>
    {
      using messages::MessageConveyor<MessageContext>::MessageConveyor;
    };
  }

  namespace client
  {
    void forward_message(const MessageContext& context, const cup::messages::RegistrationSuccess&);

    void forward_message(const MessageContext& context, const cup::messages::Intermission& intermission);
    void forward_message(const MessageContext& context, const cup::messages::Initialization& initialization);
    void forward_message(const MessageContext& context, const cup::messages::StageBegin& stage_begin);
    void forward_message(const MessageContext& context, const cup::messages::StageEnd& stage_end);
    void forward_message(const MessageContext& context, const cup::messages::CupEnd& cup_end);
    void forward_message(const MessageContext& context, const cup::messages::Restart& restart);

    // Not technically a server->client message.
    void forward_message(const MessageContext& context, const stage::messages::StageLoaded& stage_loaded);
  }
}

#endif