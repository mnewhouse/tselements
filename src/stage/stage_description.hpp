/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef STAGE_DESCRIPTION_HPP_3344676
#define STAGE_DESCRIPTION_HPP_3344676

#include "resources/car_definition.hpp"
#include "resources/track_reference.hpp"
#include "resources/color_scheme.hpp"

#include <vector>

namespace ts
{
  namespace stage
  {
    namespace object_description
    {
      struct Car
      {
        std::uint8_t instance_id;
        std::uint8_t model_id;
        std::uint16_t controller_id;
        std::uint16_t slot_id;
        std::uint16_t start_pos;

        resources::ColorScheme color_scheme;
      };
    }

    // This structure holds the description of a stage. That is, it contains a reference to
    // the track that is to be loaded, a description of all models that are to be loaded,
    // and all entities that are to be created at initialization.
    struct StageDescription
    {
      resources::TrackReference track;

      std::vector<resources::CarDefinition> car_models;
      std::vector<object_description::Car> car_instances;
    };
  }
}

#endif