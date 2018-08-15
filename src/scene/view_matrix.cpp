/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "view_matrix.hpp"
#include "viewport.hpp"

namespace ts
{
  namespace scene
  {
    sf::Transform compute_view_matrix(const Viewport& viewport, Vector2i world_size, double frame_progress)
    {
      const auto& camera = viewport.camera();
      auto screen_rect = viewport.screen_rect();
      auto area_size = make_vector2(screen_rect.width, screen_rect.height);
      
      auto center = vector2_cast<float>(compute_camera_center(camera, vector2_cast<double>(world_size),
                                                              vector2_cast<double>(area_size), frame_progress));

      auto zoom_level = static_cast<float>(camera.zoom_level());
      auto rotation = static_cast<float>(camera.rotation().degrees());

      sf::Vector2f scale(2.0f * zoom_level / area_size.x, -2.0f * zoom_level / area_size.y);
      sf::Vector2f translation(-center.x, -center.y);      
      return sf::Transform().rotate(rotation).scale(scale).translate(translation);
    }
  }
}