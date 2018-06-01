/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "utility/vector3.hpp"
#include "utility/rotation.hpp"

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace ts
{
  namespace scene3d
  {
    class Camera
    {
    public:
      enum Mode
      {
        Fixed,
        Follow_FixedOffset,
        Follow_FixedPosition
      };

      enum ViewType
      {
        Orthographic,
        Perspective
      };

      glm::mat4 matrix() const;

      void set_mode(Mode mode);
      Mode mode() const;

      void set_view_type(ViewType type);
      ViewType view_type() const;

      void set_field_of_view(Rotation<float> fov);
      Rotation<float> field_of_view() const;

      void set_position(Vector3f position);
      Vector3f position() const;

      void set_look_at(Vector3f look_at);
      Vector3f look_at() const;      

    private:
      glm::vec3 position_ = { 1050.0f, 1050.0f, 1500.0f };
      glm::vec3 look_at_ = { 1000.0f, 1000.0f, 0.0f };
      Rotation<float> field_of_view_ = degrees(60.f);

      Mode mode_ = Fixed;

      ViewType view_type_ = Perspective;
    };
  }
}