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
      void forward_to(CupSynchronizer& synchronizer, const MessageType& message)
      {
        synchronizer.handle_message(message);
      }
    }

    template <typename MessageDispatcherType>
    template <typename MessageType>
    void CupController<MessageDispatcherType>::dispatch_message(MessageType&& message)
    {
      // First, synchronize our own cup according to the message
      detail::forward_to(cup_synchronizer_, std::forward<MessageType>(message));

      // Then, tell the outside world about it.
      message_dispatcher_(std::forward<MessageType>(message));     
    }

    template <typename MessageDispatcher>
    void CupController<MessageDispatcher>::advance()
    {
      const auto state = cup_.cup_state();

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
      // Send an initialization message, and wait for all clients to send a ready message back.
      std::fill(client_ready_states_.begin(), client_ready_states_.end(), 0);

      dispatch_message(cup::messages::make_initialization_message(stage_desc));      
    }

    template <typename MessageDispatcher>
    void CupController<MessageDispatcher>::begin_cup()
    {
      const auto& tracks = cup_.tracks();

      if (tracks.empty())
      {
        // Oops, the cup is over before it even began.
        end_cup();
      }

      else
      {
        // Otherwise, dispatch an intermission event.
        intermission();
      }
    }

    template <typename MessageDispatcher>
    void CupController<MessageDispatcher>::intermission()
    {
      auto stage_id = cup_.current_stage();
      const auto& tracks = cup_.tracks();

      // Gather information about the next track, then just dispatch a message based on it.
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

      // This one is a bit tricky. We have to pass around a local initialization message
      // to initiate the loading process, but we have to wait until the loading is finished
      // before we can dispatch the actual initialization message to the clients.
      messages::PreInitialization pre_initialization;
      pre_initialization.stage_id = stage_id;

      auto& stage_description = pre_initialization.stage_description;      
      stage_description.track = tracks[stage_id];
      stage_description.car_models = cup_.cars();

      // Give everyone a car. We're being particularly generous today.
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
          car.start_pos = 0;

          stage_description.car_instances.push_back(car);
        }
      }
      
      for (std::uint32_t x = 1; x != 5; ++x)
      {
        stage::object_description::Car car;
        car.controller_id = 0xFFFF;
        car.instance_id = instance_id++;
        car.model_id = 0;
        car.slot_id = 0;
        car.start_pos = instance_id;
        stage_description.car_instances.push_back(car);
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

      // Dispatch an event based on whether there are any stages left.
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
        // Once we have received a ready message from everyone, begin the stage.
        client_ready_states_[client_id] = 1;

        if (is_everyone_ready())
        {
          begin_stage();
        }
      }
    }
  }
}