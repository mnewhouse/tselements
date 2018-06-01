/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "viewport.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

namespace ts
{
  namespace scene
  {
    glm::mat4 compute_view_matrix(const Viewport& viewport, Vector2i world_size, double frame_progress)
    {
      const auto& camera = viewport.camera();
      auto screen_rect = viewport.screen_rect();
      auto area_size = make_vector2(screen_rect.width, screen_rect.height);

      auto zoom_level = camera.zoom_level();
      auto center = compute_camera_center(camera, vector2_cast<double>(world_size),
                                          vector2_cast<double>(area_size), frame_progress);

      glm::vec3 scale(2.0 * zoom_level / area_size.x, -2.0f * zoom_level / area_size.y, 0.0);
      glm::vec3 translation(-center.x, -center.y, 0);

      const glm::vec3 rotation_axis(0, 0, 1);
      auto rotation = static_cast<float>(camera.rotation().radians());

      return glm::translate(glm::scale(glm::rotate(rotation, rotation_axis), scale), translation);
    }

    glm::mat4 compute_inverse_view_matrix(const Viewport& viewport, Vector2i world_size, double frame_progress)
    {
      const auto& camera = viewport.camera();
      auto screen_rect = viewport.screen_rect();
      auto area_size = make_vector2(screen_rect.width, screen_rect.height);

      auto zoom_level = camera.zoom_level();
      auto center = compute_camera_center(camera, vector2_cast<double>(world_size),
                                          vector2_cast<double>(area_size), frame_progress);

      glm::vec3 scale(area_size.x / (2.0 * zoom_level), -area_size.y / (2.0 * zoom_level), 0);
      glm::vec3 translation(center.x, center.y, 0);
      
      const glm::vec3 rotation_axis(0, 0, 1);
      auto rotation = static_cast<float>(-camera.rotation().radians());

      return glm::rotate(glm::scale(glm::translate(translation), scale), rotation, rotation_axis);
    }
  }
}