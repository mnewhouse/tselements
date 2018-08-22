/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "editor_control_points_mode.hpp"

#include "editor/editor_context.hpp"
#include "editor/editor_scene.hpp"
#include "editor/editor_action_history.hpp"

#include "resources/track.hpp"

#include "imgui/imgui.h"

namespace ts
{
  namespace editor
  {
    const char* ControlPointsMode::mode_name() const
    {
      return "Control Points";
    }

    void ControlPointsMode::update_tool_info(const EditorContext& context)
    {

    }

    void ControlPointsMode::activate(const EditorContext& context)
    {
      selected_point_id_ = -1;
    }

    void ControlPointsMode::next(const EditorContext& context)
    {
      auto& cps = context.scene.track().control_points();
      if (selected_point_id_ + 1 < cps.size())
      {
        ++selected_point_id_;
      }     
    }

    void ControlPointsMode::previous(const EditorContext& context)
    {
      if (selected_point_id_ > 0) --selected_point_id_;
    }

    
    void ControlPointsMode::update_canvas_interface(const EditorContext& context)
    {
      auto& io = ImGui::GetIO();
      auto& track = context.scene.track();

      auto canvas_pos = ImGui::GetWindowPos();
      auto mouse_pos = ImGui::GetMousePos();
      auto canvas_mouse_pos = make_vector2<double>(mouse_pos.x - canvas_pos.x,
                                                   mouse_pos.y - canvas_pos.y);

      CoordTransform transform(context);
      auto world_pos = vector2_cast<std::int32_t>(transform.world_position(canvas_mouse_pos));

      auto screen_pos = [&](auto world_pos)
      {
        auto p = transform.viewport_position(vector2_cast<double>(world_pos));
        return ImVec2(p.x + canvas_pos.x, p.y + canvas_pos.y);
      };

      using resources::ControlPoint;

      auto draw_list = ImGui::GetWindowDrawList();
      auto preview_color = ImColor(1.0f, 1.0f, 1.0f, 1.0f);
      auto point_color = ImColor(1.0f, 1.0f, 0.6f, 1.0f);
      
      std::uint32_t sector_id = 0;
      std::uint32_t point_id = 1;

      const auto& cps = track.control_points();
      for (const auto& cp : cps)
      {
        auto c = (point_id == selected_point_id_ ? preview_color : point_color);

        auto p1 = screen_pos(cp.start);
        auto p2 = screen_pos(cp.end);

        draw_list->AddLine(p1, p2, c);
        
        char buffer[32] = "Finish Line";        
        if ((cp.flags & ControlPoint::Sector) != 0)
        {
          std::sprintf(buffer, "S%u", sector_id);          

          ++sector_id;
        }

        else if (point_id != 1)
        {
          std::sprintf(buffer, "%u", point_id);
        }

        ++point_id;

        auto text_pos = ImVec2((p1.x + p2.x) * 0.5f + 5.0f,
          (p1.y + p2.y) * 0.5f + 5.0f);
        draw_list->AddText(text_pos, c, buffer);
      }

      if (context.canvas_focus && ImGui::IsMouseClicked(0) && !added_point_position_)
      {
        added_point_position_ = world_pos;       
      }

      else if (context.canvas_focus && added_point_position_)
      {
        auto idx = std::min<std::size_t>(selected_point_id_, cps.size());

        // Draw preview line
        ControlPoint cp;
        cp.start = *added_point_position_;
        cp.end = world_pos;
        cp.flags = 0;

        auto diff = world_pos - *added_point_position_;
        if (io.KeyCtrl)
        {
          cp.type = ControlPoint::Type::Arbitrary;
        }

        else
        {
          if (std::abs(cp.start.x - cp.end.x) > std::abs(cp.start.y - cp.end.y))
          {
            cp.type = ControlPoint::HorizontalLine;
            cp.end.y = cp.start.y;
          }

          else
          {
            cp.type = ControlPoint::VerticalLine;
            cp.end.x = cp.start.x;
          }          
        }

        draw_list->AddLine(screen_pos(cp.start), screen_pos(cp.end), preview_color);
        if (ImGui::IsMouseClicked(0) && cp.start != cp.end)
        {
          // Add control point.
          auto action = [=]()
          {
            context.scene.track().add_control_point(cp, idx);
            selected_point_id_ = idx + 1;
          };

          auto undo_action = [=]()
          {
            context.scene.track().remove_control_point(idx);
          };

          context.action_history.push_action("Add CP", action, undo_action);

          added_point_position_ = boost::none;
        }
      }
    }


    void ControlPointsMode::delete_last(const EditorContext& context)
    {
      auto& track = context.scene.track();
      auto& cps = track.control_points();

      auto idx = std::min<std::size_t>(selected_point_id_, cps.size()) - 1;      
      if (idx < cps.size())
      {
        auto& point = cps[idx];
        auto action = [=]()
        {
          context.scene.track().remove_control_point(idx);
          selected_point_id_ = idx;
        };

        auto undo_action = [=]()
        {
          context.scene.track().add_control_point(point, idx);
        };

        context.action_history.push_action("Remove CP", action, undo_action);
      }
    }

    void ControlPointsMode::delete_selected(const EditorContext& context)
    {
      auto& track = context.scene.track();
      auto& cps = track.control_points();

      auto idx = std::min<std::size_t>(selected_point_id_, cps.size());
      if (idx > 0)
      {
        auto& point = cps[idx];
        auto action = [=]()
        {
          context.scene.track().remove_control_point(idx - 1);
          selected_point_id_ = std::min(idx, context.scene.track().control_points().size());
        };

        auto undo_action = [=]()
        {
          context.scene.track().add_control_point(point, idx - 1);
        };

        context.action_history.push_action("Remove CP", action, undo_action);
      }
    }
  }
}