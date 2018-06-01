/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "cup.hpp"
#include "cup_settings.hpp"

#include <algorithm>

namespace ts
{
  namespace cup
  {
    Cup::Cup(const CupSettings& cup_settings)
      : tracks_(cup_settings.tracks),
        cars_(cup_settings.selected_cars),
        car_mode_(cup_settings.car_mode),
        max_player_count_(cup_settings.max_players)
    {
    }

    void Cup::add_track(const resources::TrackReferenceView& track_ref)
    {
      tracks_.emplace_back(track_ref);
    }

    void Cup::remove_track(const resources::TrackReferenceView& track_ref)
    {
      auto end = std::remove_if(tracks_.begin(), tracks_.end(), 
                                [track_ref](const resources::TrackReference& entry)
      {
        return track_ref.path == entry.path;
      });

      tracks_.erase(end, tracks_.end());
    }

    const std::vector<resources::TrackReference>& Cup::tracks() const
    {
      return tracks_;
    }

    void Cup::add_car(const resources::CarDefinition& car_desc)
    {
      cars_.push_back(car_desc);
    }

    void Cup::remove_car(const resources::CarDescriptionRef& car_desc)
    {
      auto end = std::remove_if(cars_.begin(), cars_.end(),
                                [car_desc](const resources::CarDefinition& entry)
      {
        return entry.car_name == car_desc.name && entry.car_hash == car_desc.hash;
      });

      cars_.erase(end, cars_.end());
    }

    const std::vector<resources::CarDefinition>& Cup::cars() const
    {
      return cars_;
    }

    void Cup::set_car_mode(CarMode car_mode)
    {
      car_mode_ = car_mode;
    }

    CarMode Cup::car_mode() const
    {
      return car_mode_;
    }

    CupState Cup::cup_state() const
    {
      return cup_state_;
    }

    void Cup::set_cup_state(CupState cup_state)
    {
      cup_state_ = cup_state;
    }

    std::uint32_t Cup::current_stage() const
    {
      return current_stage_;
    }

    void Cup::set_current_stage(std::uint32_t stage)
    {
      current_stage_ = stage;
    }

    void Cup::advance_stage()
    {
      ++current_stage_;
    }

    void Cup::restart()
    {
      current_stage_ = 0;
      cup_state_ = CupState::Registration;
    }

    std::pair<std::uint16_t, RegistrationStatus>
      Cup::register_client(const PlayerDefinition* players, std::size_t player_count)
    {
      if (player_count != 0 && player_count_ + player_count >= max_player_count_)
      {
        return std::make_pair(invalid_client_id, RegistrationStatus::TooManyPlayers);
      }

      auto it = std::find_if(clients_.begin(), clients_.end(), 
                   [](const auto& optional_client)
      {
        return optional_client == boost::none;
      });

      if (it == clients_.end())
      {
        return std::make_pair(invalid_client_id, RegistrationStatus::TooManyClients);
      }
      
      it->emplace();
      auto& client = **it;

      client.players.assign(players, players + player_count);
      player_count_ += player_count;

      client.id = static_cast<std::uint16_t>(std::distance(clients_.begin(), it));
      if (client.id >= clients_end_) clients_end_ = client.id + 1;

      return std::make_pair(client.id, RegistrationStatus::Success);
    }

    void Cup::unregister_client(std::uint16_t client_id)
    {
      clients_[client_id] = boost::none;

      if (clients_end_ - 1 == client_id)
      {
        while (clients_end_ != 0 && clients_[client_id--] == boost::none)
        {
          --clients_end_;
        }
      }
    }

    Cup::client_range Cup::clients() const
    {
      return client_range(clients_.data(), clients_.data() + clients_end_);
    }
  }
}