/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "camera_3d.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace ts
{
  namespace scene3d
  {
    glm::mat4 Camera::matrix() const
    {
      return glm::lookAt(position_, look_at_, glm::vec3(0.f, 1.f, 0.f));
    }

    Camera::Mode Camera::mode() const
    {
      return mode_;
    }

    void Camera::set_mode(Mode mode)
    {
      mode_ = mode;
    }

    void Camera::set_position(Vector3f pos)
    {
      position_ = { pos.x, pos.y, pos.z };
    }

    Vector3f Camera::position() const
    {
      return{ position_.x, position_.y, position_.z };
    }

    void Camera::set_look_at(Vector3f pos)
    {
      look_at_ = { pos.x, pos.y, pos.z };
    }

    Vector3f Camera::look_at() const
    {
      return{ look_at_.x, look_at_.y, look_at_.z };
    }

    void Camera::set_view_type(ViewType type)
    {
      view_type_ = type;
    }

    Camera::ViewType Camera::view_type() const
    {
      return view_type_;
    }

    Rotation<float> Camera::field_of_view() const
    {
      return field_of_view_;
    }

    void Camera::set_field_of_view(Rotation<float> fov)
    {
      field_of_view_ = fov;
    }
  }
}