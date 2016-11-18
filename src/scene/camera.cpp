/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"

#include "camera.hpp"

#include "world/entity.hpp"

namespace ts
{
  namespace scene
  {
    void Camera::update_position()
    {
      if (followed_entity_)
      {
        position_ = followed_entity_->position();
      }
    }

    const world::Entity* Camera::followed_entity() const
    {
      return followed_entity_;
    }

    void Camera::follow_entity(const world::Entity* entity)
    {
      followed_entity_ = entity;

      update_position();
    }

    void Camera::set_position(Vector2<double> position)
    {
      position_ = position;
      followed_entity_ = nullptr;
    }

    void Camera::set_rotation(Rotation<double> rotation)
    {
      rotation_ = rotation;
      followed_entity_ = nullptr;
    }

    void Camera::set_zoom_level(double zoom_level)
    {
      zoom_level_ = zoom_level;
    }

    Vector2<double> Camera::position() const
    {
      return position_;
    }

    Rotation<double> Camera::rotation() const
    {
      return rotation_;
    }

    double Camera::zoom_level() const
    {
      return zoom_level_;
    }

    Vector2d compute_camera_center(const Camera& camera, Vector2d world_size,
                                   Vector2d screen_size, double frame_progress)
    {
      auto zoom = camera.zoom_level(), inverse_zoom = 1.0 / zoom;
      auto position = camera.position();

      if (camera.followed_entity())
      {
        position += (camera.followed_entity()->position() - position) * frame_progress;
      }

      // If edges of the screen are outside the world bounds, move the center.
      // Unless the world is smaller than the visible area, in which case
      // we center the thing.

      auto center = position;
      if (world_size.x * zoom < screen_size.x)
      {
        center.x = world_size.x * 0.5;
      }

      else
      {
        auto left_edge = center.x - (screen_size.x * 0.5) * inverse_zoom;
        auto right_edge = center.x + (screen_size.x * 0.5) * inverse_zoom;

        if (left_edge < 0.0)
        {
          center.x -= left_edge;
        }

        else if (right_edge >= world_size.x)
        {
          center.x -= (right_edge - world_size.x);
        }
      }

      if (world_size.y * zoom < screen_size.y)
      {
        center.y = world_size.y * 0.5;
      }

      else
      {
        auto top_edge = center.y - (screen_size.y * 0.5) * inverse_zoom;
        auto bottom_edge = center.y + (screen_size.y * 0.5) * inverse_zoom;
        if (top_edge < 0.0)
        {
          center.y -= top_edge;
        }

        else if (bottom_edge >= world_size.y)
        {
          center.y -= (bottom_edge - world_size.y);
        }
      }

      return center;
    }
  }
}