/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "cup_state.hpp"
#include "player_definition.hpp"

#include "resources/track_description.hpp"
#include "resources/track_reference.hpp"
#include "resources/car_description.hpp"

#include "stage/stage_description.hpp"

#include <boost/container/small_vector.hpp>

#include <cstdint>
#include <vector>

namespace ts
{
  namespace stage
  {
    struct StageDescription;
  }

  namespace cup
  {
    struct StageCarDescription
    {
      std::uint8_t model_id;
      std::uint8_t instance_id;
      std::uint16_t controller_id;
    };

    namespace messages
    {
      struct RegistrationRequest
      {
        boost::container::small_vector<PlayerDefinition, 8> players;
      };      

      struct RegistrationSuccess
      {
        std::uint64_t client_key;
        std::uint16_t client_id;
      };

      struct ServerFull
      {
      };

      struct Intermission
      {
        std::uint32_t stage_id;
        resources::TrackDescriptionView track_desc;
      };

      struct PreInitialization
      {
        std::uint32_t stage_id;
        stage::StageDescription stage_description;
      };

      struct Initialization
      {
        // Description of the stage that is to be loaded.
        resources::TrackDescription track;
        std::vector<resources::CarDescription> car_models;
        std::vector<StageCarDescription> car_instances;
      };

      struct StageBegin
      {
        std::uint32_t stage_id;
      };

      struct StageEnd
      {
        std::uint32_t stage_id;
      };

      struct CupEnd
      {
      };

      struct Advance
      {
      };

      struct Restart
      {
      };

      struct Ready
      {
      };

      struct StageBeginRequest
      {
      };

      struct ClientJoined
      {
        boost::container::small_vector<PlayerDefinition, 8> players;
      };

      Initialization make_initialization_message(const stage::StageDescription& stage_desc);
    }
  }
}
