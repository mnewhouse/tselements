/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "local_player_controller.hpp"

#include "controls/control_center.hpp"

#include "cup/cup_messages.hpp"

#include "stage/stage_description.hpp"

namespace ts
{
  namespace client
  {
    LocalPlayerController::LocalPlayerController(const cup::PlayerDefinition* players, 
                                                 std::size_t player_count)
    {
      local_players_.assign(players, players + player_count);
    }

    void LocalPlayerController::handle_message(const cup::messages::RegistrationSuccess& success)
    {
      client_id_ = success.client_id;
    }

    controls::ControlCenter 
      LocalPlayerController::create_control_center(const stage::StageDescription& stage_desc) const
    {
      controls::ControlCenter control_center;
      for (auto& instance : stage_desc.car_instances)
      {
        if (instance.controller_id != client_id_) continue;

        auto it = std::find_if(local_players_.begin(), local_players_.end(),
                               [=](const cup::PlayerDefinition& local_player)
        {
          return local_player.control_slot == instance.slot_id;
        });

        if (it != local_players_.end())
        {
          control_center.assume_control(instance.slot_id, controls::Controllable(instance.instance_id));
        }
      }

      return control_center;
    }
  }
}