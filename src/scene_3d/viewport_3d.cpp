/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "viewport_3d.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace ts
{
  namespace scene3d
  {
    Viewport::Viewport(IntRect screen_rect)
      : screen_rect_(screen_rect)
    {
    }

    void Viewport::set_screen_rect(IntRect rect)
    {
      screen_rect_ = rect;
    }

    IntRect Viewport::screen_rect() const
    {
      return screen_rect_;
    }

    const Camera& Viewport::camera() const
    {
      return camera_;
    }

    Camera& Viewport::camera()
    {
      return camera_;
    }

    glm::mat4 projection_matrix(const Viewport& view_port)
    {
      const auto& camera = view_port.camera();
      auto screen_rect = rect_cast<float>(view_port.screen_rect());
      auto fov = camera.field_of_view();
      auto aspect = screen_rect.width / screen_rect.height;

      if (camera.view_type() == Camera::ViewType::Perspective)
      {       
        auto mat = glm::perspective(fov.radians(), aspect, 2.0f, 5000.0f);
        
        return glm::scale(mat, glm::vec3(1.0f, -1.0f, 1.0f)) * camera.matrix();
      }

      else
      {
        auto offset = camera.look_at() - camera.position();
        auto factor = std::tan(fov.radians() * 0.5f);        
        auto s = magnitude(offset) * factor;

        auto mat = glm::ortho(-s * aspect, s * aspect, s, -s, -2500.f, 2500.0f);
        return mat * camera.matrix();
      }
    }

    glm::mat4 inverse_projection_matrix(const Viewport& view_port)
    {
      return glm::inverse(projection_matrix(view_port));
    }
  }
}