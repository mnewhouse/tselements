/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "control_point.hpp"
#include "start_point.hpp"

#include "utility/vector2.hpp"
#include "utility/rotation.hpp"

#include <cmath>

namespace ts
{
  namespace resources
  {
    // This function can be used to generate a default starting grid based on a given finish line.
    template <typename OutIt>
    void generate_default_start_points(const ControlPoint& finish_line, std::size_t num_points, 
                                       std::uint32_t grid_spacing, OutIt out)
    {
      auto start = vector2_cast<double>(finish_line.start);
      auto end = vector2_cast<double>(finish_line.end);
      auto center = start + (end - start) * 0.5;
      
      auto grid_direction = flip_orientation(normalize(end - start));

      // The dominant direction should be either to the left or to the bottom.
      if (std::abs(grid_direction.x) < std::abs(grid_direction.y))
      {
        if (grid_direction.y > 0.0) grid_direction = -grid_direction;
      }

      else if (grid_direction.x > 0.0) grid_direction = -grid_direction;

      auto spacing = static_cast<double>(grid_spacing);
      auto lateral_offset = make_vector2(grid_direction.y, -grid_direction.x) * spacing;

      auto left_column_start = center - lateral_offset + grid_direction * (3.0 + spacing);
      auto right_column_start = center + lateral_offset + grid_direction * (3.0 + spacing * 2.0);

      auto rotation = radians(std::atan2(-grid_direction.x, grid_direction.y));

      StartPoint start_point;      
      start_point.rotation = static_cast<std::int32_t>(rotation.degrees());

      auto increment = grid_direction * spacing * 2.0;
      auto offset = make_vector2(0.0, 0.0);

      // Now, add as many points as are needed, alternating between 
      // the left and right column.
      for (std::size_t i = 0; i != num_points; ++i, ++out)
      {
        if ((i & 1) == 0)
        {
          start_point.position = vector2_round<std::int32_t>(left_column_start + offset);
        }

        else
        {
          start_point.position = vector2_round<std::int32_t>(right_column_start + offset);
          offset += grid_direction * spacing * 2.0;
        }

        *out = start_point; 
      }
    }
  }
}
