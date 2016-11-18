/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "resources/control_point.hpp"

#include <vector>
#include <cstdint>

namespace ts
{
  namespace world
  {
    using ControlPointId = std::uint32_t;

    struct ControlPoint
      : resources::ControlPoint
    {
      ControlPointId id;
    };

    class ControlPointManager
    {
    public:
      ControlPointManager(const resources::ControlPoint* persistent_points, std::size_t persistent_point_count);

      const std::vector<ControlPoint>& control_points() const;

      ControlPointId create_control_point(const resources::ControlPoint& point);
      void destroy_control_point(ControlPointId point_id);

      template <typename EventCallback>
      void test_control_point_intersections(Vector2<double> old_position, Vector2<double> new_position,
                                            EventCallback&& event_callback);

    private:
      std::vector<ControlPoint> control_points_;
      std::size_t persistent_point_count_;
    };
  }
}
