/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef COLLISIONS_HPP_85918192495
#define COLLISIONS_HPP_85918192495

#include "utility/vector2.hpp"

#include "resources/collision_mask.hpp"

namespace ts
{
  namespace world
  {
    class Entity;

    struct CollisionResult
    {
      CollisionResult() = default;

      Vector2i point;
      Vector2<double> normal;
      double impact;
      double bounce_factor;
    };

    using resources::CollisionMask;
    using resources::CollisionMaskFrame;

    CollisionResult examine_scenery_collision(const CollisionMaskFrame& scenery, Vector2i global_point,
                                              Vector2<double> subject_velocity, Vector2<double> entry_vector,
                                              double bounce_factor);

    void resolve_scenery_collision(const CollisionResult& collision, Entity& entity,
                                   Rotation<double> rotation_delta);
  }
}

#endif