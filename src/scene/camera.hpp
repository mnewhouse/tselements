/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <cstddef>

#include "utility/vector2.hpp"
#include "utility/rect.hpp"
#include "utility/rotation.hpp"

namespace ts
{
  namespace world
  {
    class Entity;
  }

  namespace scene
  {
    // The Camera class represents an in-game camera. It can follow an entity around,
    // but it also has an interface to set position, rotation or zoom level manually,
    // which will cause the entity to stop being followed.
    class Camera
    {
    public:
      void update_position();

      void follow_entity(const world::Entity* entity);
      const world::Entity* followed_entity() const;

      void set_position(Vector2<double> position);
      void set_zoom_level(double zoom_level);
      void set_rotation(Rotation<double> rotation);

      Vector2<double> position() const;
      double zoom_level() const;
      Rotation<double> rotation() const;

    private:
      const world::Entity* followed_entity_ = nullptr;
      Vector2<double> position_;
      Rotation<double> rotation_; 
      double zoom_level_ = 2.0;
    };

    // This function computes the center position of the camera. If the view area stretches out into
    // the great void outside the game world, the view will be clamped to the edge.
    // Additionally, if the view area is larger than the game world in any dimension, the view will
    // be centered.
    Vector2<double> compute_camera_center(const Camera& camera, Vector2d world_size, 
                                          Vector2d screen_size, double frame_progress);
  }
}
