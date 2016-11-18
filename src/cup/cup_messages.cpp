/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"
#include "cup_messages.hpp"

#include "stage/stage_description.hpp"

#include <algorithm>

namespace ts
{
  namespace cup
  {
    namespace messages
    {
      Initialization make_initialization_message(const stage::StageDescription& stage_desc)
      {
        Initialization initialization;
        initialization.track.name = stage_desc.track.name;
        initialization.track.hash = stage_desc.track.hash;

        // Mostly this just copies the stuff over.
        initialization.car_models.resize(stage_desc.car_models.size());
        std::transform(stage_desc.car_models.begin(), stage_desc.car_models.end(), initialization.car_models.begin(),
                       [](const auto& model)
        {
          resources::CarDescription car_desc;
          car_desc.hash = model.car_hash;
          car_desc.name = model.car_name;
          return car_desc;
        });

        initialization.car_instances.resize(stage_desc.car_instances.size());
        std::transform(stage_desc.car_instances.begin(), stage_desc.car_instances.end(), initialization.car_instances.begin(),
                       [](const auto& instance)
        {
          StageCarDescription entry;
          entry.model_id = instance.model_id;
          entry.instance_id = instance.instance_id;
          entry.controller_id = instance.controller_id;
          return entry;
        });

        return initialization;
      }
    }
  }
}