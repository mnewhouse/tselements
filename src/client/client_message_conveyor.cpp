/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "client_message_conveyor.hpp"
#include "client_scene_loader.hpp"
#include "cup_state_interface.hpp"

#include "cup/cup_synchronizer.hpp"

namespace ts
{
  namespace client
  {
    namespace detail
    {
      template <typename HandlerType, typename MessageType>
      auto forward(HandlerType& handler, const MessageType& message, int)
        -> decltype(handler.handle_message(message), void())
      {
        handler.handle_message(message);
      }

      template <typename HandlerType, typename MessageType>
      void forward(HandlerType& handler, const MessageType& message, void*)
      {
      }

      template <typename HandlerType, typename MessageType>
      void forward(HandlerType& handler, const MessageType& message)
      {
        forward(handler, message, 0);
      }

      template <typename MessageType>
      void forward(const MessageContext& context, const MessageType& message)
      {
        if (context.cup_state_interface) forward(*context.cup_state_interface, message);
        if (context.cup_synchronizer) forward(*context.cup_synchronizer, message);
        if (context.scene_loader) forward(*context.scene_loader, message);
        if (context.local_player_controller) forward(*context.local_player_controller, message);
      }
    }

    void forward_message(const MessageContext& context, const cup::messages::RegistrationSuccess& success)
    {
      detail::forward(context, success);
    }
    
    void forward_message(const MessageContext& context, const cup::messages::Intermission& intermission)
    {
      detail::forward(context, intermission);
    }

    void forward_message(const MessageContext& context, const cup::messages::Initialization& initialization)
    {
      detail::forward(context, initialization);
    }

    void forward_message(const MessageContext& context, const cup::messages::StageBegin& stage_begin)
    {
      detail::forward(context, stage_begin);
    }

    void forward_message(const MessageContext& context, const cup::messages::StageEnd& stage_end)
    {
      detail::forward(context, stage_end);
    }

    void forward_message(const MessageContext& context, const cup::messages::CupEnd& cup_end)
    {
      detail::forward(context, cup_end);
    }

    void forward_message(const MessageContext& context, const cup::messages::Restart& restart)
    {
      detail::forward(context, restart);
    }

    void forward_message(const MessageContext& context,
                         const cup::messages::PreInitialization& pre_initialization)
    {
      detail::forward(context, pre_initialization);
    }

    void forward_message(const MessageContext& context,
                         const stage::messages::StageLoaded& stage_loaded)
    {
      detail::forward(context, stage_loaded);
    }
  }
}