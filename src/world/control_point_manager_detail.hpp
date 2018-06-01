/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "control_point_manager.hpp"
#include "entity.hpp"

#include "utility/line_intersection.hpp"

namespace ts
{
  namespace world
  {
    namespace detail
    {
      template <typename EventCallback>
      void test_finish_line_intersection(Vector2<double> old_position, Vector2<double> new_position,
                                         const ControlPoint& point, EventCallback&& event_callback)
      {
        auto intersection = find_line_segment_intersection(old_position, new_position,
                                                           vector2_cast<double>(point.start),
                                                           vector2_cast<double>(point.end));
        if (intersection)
        {
          auto x_diff = new_position.x - old_position.x;
          auto y_diff = new_position.y - old_position.y;

          double time_point = std::abs(x_diff) < std::abs(y_diff) ?
            (intersection.point.y - old_position.y) / y_diff :
            (intersection.point.x - old_position.x) / x_diff;

          event_callback(point, time_point);
        }
      }

      template <typename EventCallback>
      void test_vertical_line_intersection(Vector2<double> old_position, Vector2<double> new_position,
                                           const ControlPoint& point, EventCallback&& event_callback)
      {
        auto x = static_cast<double>(point.start.x);
        if (old_position.x < x != new_position.x < x)
        {
          auto time_point = (x - old_position.x) / (new_position.x - old_position.x);
          auto intersect_y = static_cast<std::int32_t>(old_position.y + time_point * (new_position.y - old_position.y));

          auto y = std::minmax(point.start.y, point.end.y);
          if (intersect_y >= y.first && intersect_y <= y.second)
          {
            event_callback(point, time_point);
          }
        }
      }

      template <typename EventCallback>
      void test_horizontal_line_intersection(Vector2<double> old_position, Vector2<double> new_position,
                                             const ControlPoint& point, EventCallback&& event_callback)
      {
        auto y = static_cast<double>(point.start.y);
        if (old_position.y < y != new_position.y < y)
        {
          auto time_point = (y - old_position.y) / (new_position.y - old_position.y);
          auto intersect_x = static_cast<std::int32_t>(old_position.x + time_point * (new_position.x - old_position.x));

          auto x = std::minmax(point.start.x, point.end.x);
          if (intersect_x >= x.first && intersect_x <= x.second)
          {
            event_callback(point, time_point);
          }
        }
      }

      template <typename EventCallback>
      void test_area_intersection(Vector2<double> old_position, Vector2<double> new_position,
                                  const ControlPoint& point, EventCallback&& event_callback)
      {
        // Test if the trajectory passes into the control area specified by "area",
        // and possibly out of it again.
        auto area = make_rect_from_points(vector2_cast<double>(point.start), vector2_cast<double>(point.end));
        // TODO

      }
    }

    template <typename EventCallback>
    void ControlPointManager::test_control_point_intersections(Vector2<double> old_position, Vector2<double> new_position, 
                                                               EventCallback&& event_callback)
    {      
      for (const auto& point : control_points_)
      {
        switch (point.type)
        {
        case ControlPoint::FinishLine:
          detail::test_finish_line_intersection(old_position, new_position, point, event_callback);
          break;

        case ControlPoint::HorizontalLine:
          detail::test_horizontal_line_intersection(old_position, new_position, point, event_callback);
          break;

        case ControlPoint::VerticalLine:
          detail::test_vertical_line_intersection(old_position, new_position, point, event_callback);
          break;

        case ControlPoint::Area:
          //detail::test_area_intersection(old_position, new_position, point, event_callback);
          break;

        default:
          break;
        }
      }
    }
  }
}
