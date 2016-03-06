/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CLIENT_BASE_DETAIL_HPP_2189235891823
#define CLIENT_BASE_DETAIL_HPP_2189235891823

#include "client_base.hpp"
#include "client_cup_essentials.hpp"

namespace ts
{
  namespace client
  {
    template <typename MessageDispatcher>
    ClientBase<MessageDispatcher>::ClientBase(const game::GameContext& context, MessageDispatcher message_dispatcher)
      : cup_essentials_(std::make_unique<CupEssentials<MessageDispatcher>>(context, std::move(message_dispatcher)))
    {
    }

    template <typename MessageDispatcher>
    ClientBase<MessageDispatcher>::~ClientBase() = default;

    template <typename MessageDispatcher>
    void ClientBase<MessageDispatcher>::update(std::uint32_t frame_duration)
    {
      cup_essentials_->update(frame_duration);
    }

    template <typename MessageDispatcher>
    void ClientBase<MessageDispatcher>::process_event(const game::Event& event)
    {
      cup_essentials_->process_event(event);
    }

    template <typename MessageDispatcher>
    const LocalPlayerRoster& ClientBase<MessageDispatcher>::local_players() const
    {
      return cup_essentials_->local_players();
    }

    template <typename MessageDispatcher>
    const MessageConveyor<MessageDispatcher>& ClientBase<MessageDispatcher>::message_conveyor() const
    {
      return cup_essentials_->message_conveyor();
    }
  }
}

#endif