/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "cup_controller.hpp"
#include "cup.hpp"
#include "cup_messages.hpp"

namespace ts
{
  namespace cup
  {
    template <typename MessageDispatcher>
    CupController<MessageDispatcher>::CupController(const CupSettings& cup_settings,
                                                    MessageDispatcher dispatcher)
      : cup_(cup_settings),
      cup_synchronizer_(&cup_),
      message_dispatcher_(std::move(dispatcher)),
      client_ready_states_(cup::max_client_count)
    {
    }

    namespace detail
    {
      template <typename MessageType>
      void forward_to(CupSynchronizer& synchronizer, const MessageType& message, short)
      {
      }

      template <typename MessageType>
      auto forward_to(CupSynchronizer& synchronizer, const MessageType& message, int)
        -> decltype(synchronizer.handle_message(message), void())
      {
        synchronizer.handle_message(message);
      }

      template <typename MessageType>
      void forward_to(CupSynchronizer& synchronizer, const MessageType& message)
      {
        forward_to(synchronizer, message, 0);
      }
    }

    template <typename MessageDispatcherType>
    template <typename MessageType>
    void CupController<MessageDispatcherType>::dispatch_message(MessageType&& message)
    {
      message_dispatcher_(std::forward<MessageType>(message));
      
      detail::forward_to(cup_synchronizer_, std::forward<MessageType>(message));
    }

    template <typename MessageDispatcher>
    void CupController<MessageDispatcher>::advance()
    {
      const auto state = cup_.cup_state();
      auto stage_id = cup_.current_stage();
      const auto& tracks = cup_.tracks();

      if (state == CupState::Registration)
      {
        begin_cup();
      }

      else if (state == CupState::Intermission)
      {
        preinitialize_stage();
      }

      else if (state == CupState::Initialization)
      {
        begin_stage();
      }

      else if (state == CupState::Action)
      {
        end_stage();
      }

      else if (state == CupState::End)
      {
        restart_cup();
      }
    }

    template <typename MessageDispatcher>
    void CupController<MessageDispatcher>::initialize_stage(const stage::StageDescription& stage_desc)
    {
      this->dispatch_message(cup::messages::make_initialization_message(stage_desc));

      std::fill(client_ready_states_.begin(), client_ready_states_.end(), 0);
    }

    template <typename MessageDispatcher>
    void CupController<MessageDispatcher>::begin_cup()
    {
      const auto& tracks = cup_.tracks();

      if (tracks.empty())
      {
        end_cup();
      }

      else
      {
        cup_.set_current_stage(0);
        intermission();
      }
    }

    template <typename MessageDispatcher>
    void CupController<MessageDispatcher>::intermission()
    {
      auto stage_id = cup_.current_stage();
      const auto& tracks = cup_.tracks();

      messages::Intermission intermission;
      intermission.stage_id = stage_id;
      intermission.track_desc.name = tracks[stage_id].name;
      intermission.track_desc.hash = tracks[stage_id].hash;
      dispatch_message(intermission);
    }

    template <typename MessageDispatcher>
    void CupController<MessageDispatcher>::preinitialize_stage()
    {
      auto stage_id = cup_.current_stage();
      const auto& tracks = cup_.tracks();

      cup_.set_cup_state(CupState::PreInitialization);

      // This one is a bit tricky. We have to pass around a local initialization message
      // to initiate the loading process, but we have to wait until the loading is finished
      // before we can dispatch the actual initialization message to the clients.
      messages::PreInitialization pre_initialization;
      pre_initialization.stage_id = stage_id;

      auto& stage_description = pre_initialization.stage_description;      
      stage_description.track = tracks[stage_id];
      stage_description.car_models = cup_.cars();

      std::uint8_t instance_id = 0;
      for (const auto& client : cup_.clients())
      {
        for (const auto& player : client.players)
        {
          stage::object_description::Car car;
          car.controller_id = client.id;
          car.instance_id = instance_id++;
          car.model_id = 0;
          car.slot_id = player.control_slot;

          stage_description.car_instances.push_back(car);
        }
      }

      dispatch_message(std::move(pre_initialization));
    }

    template <typename MessageDispatcher>
    void CupController<MessageDispatcher>::begin_stage()
    {
      messages::StageBegin stage_begin;
      stage_begin.stage_id = cup_.current_stage();
      dispatch_message(stage_begin);
    }
    
    template <typename MessageDispatcher>
    void CupController<MessageDispatcher>::end_stage()
    {
      messages::StageEnd stage_end;
      stage_end.stage_id = cup_.current_stage();
      dispatch_message(stage_end);

      if (cup_.current_stage() >= cup_.tracks().size())
      {
        end_cup();
      }

      else
      {
        intermission();
      }
    }

    template <typename MessageDispatcher>
    void CupController<MessageDispatcher>::end_cup()
    {
      messages::CupEnd cup_end;
      dispatch_message(cup_end);
    }

    template <typename MessageDispatcher>
    void CupController<MessageDispatcher>::restart_cup()
    {
      messages::Restart restart;
      dispatch_message(restart);
    }

    template <typename MessageDispatcher>
    const Cup& CupController<MessageDispatcher>::cup() const
    {
      return cup_;
    }

    template <typename MessageDispatcher>
    std::pair<std::uint16_t, RegistrationStatus>
      CupController<MessageDispatcher>::register_client(const PlayerDefinition* players, 
                                                        std::size_t player_count)
    {
      return cup_.register_client(players, player_count);
    }

    template <typename MessageDispatcher>
    void CupController<MessageDispatcher>::unregister_client(std::uint16_t client_id)
    {
      cup_.unregister_client(client_id);

      client_ready_states_[client_id] = 0;
    }

    template <typename MessageDispatcher>
    bool CupController<MessageDispatcher>::is_everyone_ready() const
    {
      for (const auto& client : cup_.clients())
      {
        if (!client.players.empty() && client_ready_states_[client.id] == 0) return false;
      }

      return true;
    }

    template <typename MessageDispatcher>
    void CupController<MessageDispatcher>::handle_ready_signal(std::uint16_t client_id)
    {
      if (cup_.cup_state() == cup::CupState::Initialization)
      {
        client_ready_states_[client_id] = 1;

        if (is_everyone_ready())
        {
          begin_stage();
        }
      }
    }
  }
}