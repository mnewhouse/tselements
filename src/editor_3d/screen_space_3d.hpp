/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "utility/vector2.hpp"
#include "utility/vector3.hpp"
#include "utility/rect.hpp"

#include <glm/mat4x4.hpp>

#include <boost/optional.hpp>

namespace ts
{
  namespace resources3d
  {
    class ElevationMap;
  }

  namespace editor3d
  {
    Vector2f relative_screen_position(Vector2i screen_position, Vector2i screen_size);
    Vector2f relative_screen_position(Vector2i screen_position, IntRect view_port);

    boost::optional<Vector3f> ground_position_at(Vector2f relative_screen_position, const glm::mat4& inverse_projected_view,
                                                 const resources3d::ElevationMap& elevation_map);

    boost::optional<Vector3f> ground_position_at(Vector2i screen_position, IntRect view_port,
                                                 const glm::mat4& inverse_projected_view,
                                                 const resources3d::ElevationMap& elevation_map);

    Vector2f relative_screen_position_at(Vector3f position, const glm::mat4& projected_view); 
    Vector2f screen_position_at(Vector3f position, const glm::mat4& projected_view, Vector2i screen_size);
    Vector2f screen_position_at(Vector3f position, const glm::mat4& projected_view, IntRect view_port);
  }
}