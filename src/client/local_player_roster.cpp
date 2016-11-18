/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"
#include "local_player_roster.hpp"

#include "controls/control_center.hpp"

#include "cup/cup_messages.hpp"

#include "stage/stage_description.hpp"

namespace ts
{
  namespace client
  {
    LocalPlayerRoster::LocalPlayerRoster(const cup::PlayerDefinition* players, 
                                         std::size_t player_count)
    {
      local_players_.assign(players, players + player_count);
    }

    void LocalPlayerRoster::registration_success(std::uint16_t client_id)
    {
      client_id_ = client_id;
    }

    const cup::PlayerDefinition* LocalPlayerRoster::players() const
    {
      return local_players_.data();
    }

    std::size_t LocalPlayerRoster::player_count() const
    {
      return local_players_.size();
    }

    controls::ControlCenter 
      LocalPlayerRoster::create_control_center(const stage::StageDescription& stage_desc) const
    {
      controls::ControlCenter control_center;
      for (auto& instance : stage_desc.car_instances)
      {
        // Add every player that has the same client id as we do into the mix.
        if (instance.controller_id != client_id_) continue;

        // Now find the local player that matches the entry's control slot.
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