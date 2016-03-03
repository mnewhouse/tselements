/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef LOCAL_CUP_STATE_HPP_2189124929
#define LOCAL_CUP_STATE_HPP_2189124929

#include "cup_state_base.hpp"

#include "client/client_message_conveyor.hpp"
#include "client/local_message_dispatcher.hpp"

#include "scene/scene.hpp"

#include "server/server.hpp"

namespace ts
{
  namespace resources
  {
    class ResourceStore;
  }

  namespace client
  {
    class LocalCupState
      : public CupStateBase<LocalMessageDispatcher>
    {
    public:
      explicit LocalCupState(const game_context& game_context);

      virtual void update(const update_context& context) override;

      virtual const cup::Cup& cup_object() const override;

    private:
      MessageContext make_message_context();

      server::Server server_;
      LocalMessageDispatcher message_dispatcher_;
      MessageConveyor message_conveyor_;
    };
  }
}

#endif