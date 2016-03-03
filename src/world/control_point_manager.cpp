/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "control_point_manager.hpp"

#include <algorithm>

namespace ts
{
  namespace world
  {
    ControlPointManager::ControlPointManager(const resources::ControlPoint* persistent_points,
                                             std::size_t persistent_point_count)
      : persistent_point_count_(persistent_point_count)
    {
      control_points_.reserve(persistent_point_count + 256);
      control_points_.resize(persistent_point_count);

      for (ControlPointId id = 0; id != persistent_point_count; ++id)
      {
        static_cast<resources::ControlPoint&>(control_points_[id]) = persistent_points[id];        
        control_points_[id].id = id;
      }
    }

    const std::vector<ControlPoint>& ControlPointManager::control_points() const
    {
      return control_points_;
    }

    ControlPointId ControlPointManager::create_control_point(const resources::ControlPoint& point)
    {
      control_points_.emplace_back();
      auto& back = control_points_.back();
      back.id = control_points_.size();
      back.start = point.start;
      back.end = point.end;
      back.type = point.type;
      return back.id;
    }

    void ControlPointManager::destroy_control_point(ControlPointId point_id)
    {
      if (point_id >= persistent_point_count_)
      {
        auto point_it = std::find_if(control_points_.begin() + persistent_point_count_, control_points_.end(),
                                     [=](const auto& point)
        {
          return point.id == point_id;
        });

        if (point_it != control_points_.end())
        {
          control_points_.erase(point_it);
        }
      }
    }
  }
}