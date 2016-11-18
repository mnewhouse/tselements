/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "car_mode.hpp"
#include "cup_state.hpp"
#include "player_definition.hpp"

#include "resources/track_reference.hpp"
#include "resources/car_definition.hpp"
#include "resources/car_description.hpp"

#include <boost/iterator/filter_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/optional.hpp>

#include <vector>
#include <cstdint>
#include <bitset>

namespace ts
{
  namespace cup
  {
    static constexpr std::uint16_t max_client_count = 256;
    static constexpr std::uint16_t invalid_client_id = max_client_count;

    enum class RegistrationStatus
    {
      Success,
      TooManyPlayers,
      TooManyClients
    };


    struct CupSettings;

    // The Cup class represents a cup consisting of zero or more stages.
    // This class stores the settings that are to be used in the cup, including
    // tracks, cars, scripts and other things.
    // We will also have a list of clients and their associated players here.
    class Cup
    {
    public:
      explicit Cup(const CupSettings& cup_settings);
      Cup() = default;

      void add_track(const resources::TrackReferenceView& track_ref);
      void remove_track(const resources::TrackReferenceView& track_ref);
      const std::vector<resources::TrackReference>& tracks() const;

      void set_car_mode(CarMode car_mode);
      void add_car(const resources::CarDefinition& car_def);
      void remove_car(const resources::CarDescriptionRef& car_desc);

      const std::vector<resources::CarDefinition>& cars() const;
      CarMode car_mode() const;

      std::pair<std::uint16_t, RegistrationStatus>
        register_client(const PlayerDefinition* players, std::size_t player_count);

      void unregister_client(std::uint16_t client_id);

      void set_cup_state(CupState cup_state);
      CupState cup_state() const;
      std::uint32_t current_stage() const;
      void set_current_stage(std::uint32_t stage);

      void advance_stage();
      void restart();

    private:
      struct Client
      {
        std::uint16_t id;
        boost::container::small_vector<PlayerDefinition, 4> players;
      };

      struct OptionalFilter
      {
        template <typename T>
        bool operator()(const T& v) const { return static_cast<bool>(v); }
      };

      struct OptionalDereferencer
      {
        template <typename T>
        const T& operator()(const boost::optional<T>& opt) const { return *opt; }
      };

      using client_filter_iterator = boost::filter_iterator<OptionalFilter, const boost::optional<Client>*>;
      using client_range = boost::iterator_range<
        boost::transform_iterator<OptionalDereferencer, client_filter_iterator>
      >;

    public:
      client_range clients() const;

    private:

      std::vector<resources::TrackReference> tracks_;
      std::vector<resources::CarDefinition> cars_;
      CarMode car_mode_ = CarMode::Free;
      std::size_t max_player_count_ = 1;
      std::size_t max_spectator_count_ = max_client_count;
      std::size_t player_count_ = 0;      

      std::array<boost::optional<Client>, max_client_count> clients_;
      std::size_t clients_end_ = 0;

      CupState cup_state_ = CupState::Registration;
      std::uint32_t current_stage_ = 0;
    };
  }
}
