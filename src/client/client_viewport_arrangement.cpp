/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "client_viewport_arrangement.hpp"

#include "stage/stage.hpp"
#include "controls/control_center.hpp"

#include "world/car.hpp"
#include "world/entity_id_conversion.hpp"

namespace ts
{
  namespace client
  {
    scene::ViewportArrangement make_viewport_arrangement(IntRect screen_rect,
                                                         const controls::ControlCenter& control_center,
                                                         const stage::Stage& stage_obj)
    {
      scene::ViewportArrangement viewport_arrangement(64, screen_rect);

      for (std::uint16_t slot = 0; slot != controls::max_control_slots; ++slot)
      {
        for (auto& controllable : control_center.control_slot(slot))
        {
          const auto& world = stage_obj.world();
          auto car = world.find_car(world::entity_id_to_car_id(controllable.controllable_id()));
          viewport_arrangement.add_viewport(car);
        }
      }

      if (viewport_arrangement.viewport_count() == 0)
      {
        viewport_arrangement.add_viewport(nullptr);
      }

      return viewport_arrangement;
    }
  }
}