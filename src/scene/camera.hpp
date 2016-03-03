/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CAMERA_HPP_2930192309340659
#define CAMERA_HPP_2930192309340659

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

    Vector2<double> compute_camera_center(const Camera& camera, Vector2<double> world_size, 
                                          Vector2<double> screen_size, double frame_progress);
  }
}

#endif