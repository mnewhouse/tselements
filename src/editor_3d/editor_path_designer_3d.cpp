/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "editor_path_designer_3d.hpp"
#include "editor_context_3d.hpp"
#include "screen_space_3d.hpp"
#include "editor_scene_3d.hpp"
#include "working_state_3d.hpp"

#include "resources_3d/path_vertices_3d.hpp"

#include "scene_3d/path_stroke_3d.hpp"
#include "scene_3d/viewport_3d.hpp"

#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>

namespace ts
{
  namespace editor3d
  {
    void PathDesigner::update_selection(const WorkingState& working_state)
    {
      if (selected_path_ != working_state.selected_path_layer)
      {
        selected_path_ = working_state.selected_path_layer;
        selected_path_index_ = 0;

        reload_working_path();
      }
    }

    void PathDesigner::ensure_path_layer_exists(const EditorContext& context)
    {
      if (!selected_path_)
      {
        selected_path_ = context.editor_scene.create_path_layer();        
      }

      if (selected_path_index_ >= selected_path_->paths.size())
      {
        selected_path_index_ = static_cast<std::uint32_t>(selected_path_->paths.size());
        selected_path_->paths.emplace_back();
        selected_path_->paths.back().stroke_styles.emplace_back();

        context.working_state.selected_path_layer = selected_path_;
        reload_working_path();
      }
    }

    void PathDesigner::reload_working_path()
    {
      if (selected_path_ && selected_path_index_ < selected_path_->paths.size())
      {
        working_path_ = selected_path_->paths[selected_path_index_];
      }

      else
      {
        working_path_ = {};
      }     
    }

    void PathDesigner::update_canvas_interface(const EditorContext& context)
    {
      update_selection(context.working_state);

      auto canvas_pos = ImGui::GetWindowPos();
      auto mouse_pos = ImGui::GetMousePos();
      auto draw_list = ImGui::GetWindowDrawList();
      
      const auto screen_rect = context.view_port.screen_rect();
      const auto projection = projection_matrix(context.view_port);
      const auto inverse_projection = glm::inverse(projection);

      const auto& elevation_map = context.editor_scene.elevation_map();

      const auto cell_offset = elevation_map.cell_offset();      
      const auto cell_size = elevation_map.cell_size();
      const auto grid_size = elevation_map.grid_size();

      auto whole_mouse_pos = vector2_cast<std::int32_t>(make_vector2(mouse_pos.x, mouse_pos.y));
      auto ground_pos = ground_position_at(whole_mouse_pos, screen_rect,
                                          inverse_projection, elevation_map);
      auto& nodes = working_path_.nodes;

      auto transform_pos = [&](Vector3f pos)
      {        
        auto screen_pos = screen_position_at(pos, projection, screen_rect);

        return ImVec2(screen_pos.x, screen_pos.y);
      };

      bool previously_hovered = false;

      for (const auto& node : nodes)
      {
        auto elevation = interpolate_elevation_at(elevation_map, node.position);               

        auto node_pos = transform_pos(make_3d(node.position, elevation));
        auto first_pos = transform_pos(make_3d(node.first_control, elevation));
        auto second_pos = transform_pos(make_3d(node.second_control, elevation));

        ImU32 normal_color = ImColor(255, 255, 255, 128);
        ImU32 hover_color = ImColor(255, 255, 255, 200);

        draw_list->AddLine(first_pos, second_pos, normal_color, 1.0f);

        auto do_control = [&](ImVec2 screen_pos, Vector2f world_pos,
                              NodeTransformation::Control control, float control_size)
        {
          auto diff = make_vector2(screen_pos.x - mouse_pos.x, screen_pos.y - mouse_pos.y);
          bool hover = diff.x * diff.x + diff.y * diff.y < control_size * control_size;

          auto color = normal_color;
          if (!previously_hovered && hover)
          {
            color = hover_color;
            previously_hovered = true;
          }

          draw_list->AddCircle(screen_pos, control_size, color, 12, 2.0f);

          if (!node_transformation_ && hover && ImGui::IsMouseClicked(0))
          {
            auto node_id = static_cast<std::uint32_t>(&node - nodes.data());

            NodeTransformation transformation;
            transformation.control = control;
            transformation.id = node_id;
            transformation.action = NodeTransformation::Move;
            transformation.original_state = node;

            node_transformation_.emplace(transformation);
          }
        };

        do_control(node_pos, node.position, NodeTransformation::Base, 5.0f);
        do_control(first_pos, node.first_control, NodeTransformation::First, 4.0f);
        do_control(second_pos, node.second_control, NodeTransformation::Second, 4.0f);
      }

      if (ground_pos)
      {
        auto ground_pos_2d = make_2d(*ground_pos);

        if (!node_transformation_)
        {
          if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered())
          {
            // Begin the addition of a new node.
            // Let's make sure a path layer exists first.

            ensure_path_layer_exists(context);

            Node node;
            node.first_control = node.position = node.second_control = ground_pos_2d;
            node.width = current_path_width_;
            nodes.push_back(node);

            NodeTransformation transformation{};
            transformation.original_state = nodes.back();
            transformation.action = NodeTransformation::Append;
            transformation.id = static_cast<std::uint32_t>(nodes.size() - 1);
            transformation.control = NodeTransformation::Base;

            node_transformation_.emplace(transformation);
          }
        }

        if (node_transformation_)
        {
          apply_node_transformation(ground_pos_2d);
        }
      }

      if (node_transformation_ && ImGui::IsMouseReleased(0))
      {
        finalize_node_transformation(context);

        node_transformation_ = boost::none;
      }

      if (nodes.size() >= 2)
      {
        auto inverse_cell_size = 1.0f / cell_size;

        auto node_it = nodes.begin();
        auto node_end = nodes.end();
        auto next_it = std::next(node_it);
        for (; next_it != node_end; ++node_it, ++next_it)
        {
          Vector2f positions[] = { node_it->position, node_it->second_control, next_it->first_control, next_it->position };

          auto z1 = interpolate_elevation_at(elevation_map, node_it->position);
          auto z2 = interpolate_elevation_at(elevation_map, next_it->position);

          auto p1 = transform_pos(make_3d(positions[0], z1));
          auto p2 = transform_pos(make_3d(positions[1], z1));
          auto p3 = transform_pos(make_3d(positions[2], z2));
          auto p4 = transform_pos(make_3d(positions[3], z2));          

          std::uint32_t color = ImColor(0.5f, 1.0f, 1.0f, 1.0f);
          draw_list->AddBezierCurve(p1, p2, p3, p4, color, 2.0f);
        }
      }

      for (auto p : edge_intersections_)
      {
        auto z = interpolate_elevation_at(elevation_map, p.intersect_point);
        auto sp = transform_pos(make_3d(p.intersect_point, z));

        draw_list->AddCircleFilled(sp, 3.0f, ImColor(1.0f, 1.0f, 1.0f, 0.50f), 5);   
      }

      for (auto p : contained_cell_corners_)
      {
        auto z = interpolate_elevation_at(elevation_map, p.point);
        auto sp = transform_pos(make_3d(p.point, z));

        draw_list->AddCircleFilled(sp, 3.0f, ImColor(1.0f, 0.4f, 0.0f, 1.0f), 5);
      }
    }


    void PathDesigner::apply_node_transformation(Vector2f final_position)
    {
      auto& nodes = working_path_.nodes;

      switch (node_transformation_->action)
      {
      case NodeTransformation::Append:
      {
        auto& node = nodes.back();
        node.second_control = final_position;
        node.first_control = node.position - (final_position - node.position);
        break;
      }

      case NodeTransformation::Move:
      {
        auto node_id = node_transformation_->id;
        auto control = node_transformation_->control;

        auto& node = nodes[node_id];
        if (control == NodeTransformation::Base)
        {
          auto& original_state = node_transformation_->original_state;
          auto offset = final_position - original_state.position;

          node.position = final_position;
          node.first_control = original_state.first_control + offset;
          node.second_control = original_state.second_control + offset;
        }

        else if (control == NodeTransformation::First)
        {
          node.first_control = final_position;

          auto mag = magnitude(node.position - node.second_control);
          auto offset = mag * normalize(node.first_control - node.position);

          node.second_control = node.position - offset;
        }

        else if (control == NodeTransformation::Second)
        {
          node.second_control = final_position;

          auto mag = magnitude(node.position - node.first_control);
          auto offset = mag * normalize(node.second_control - node.position);

          node.first_control = node.position - offset;
        }

        break;
      }

      default:
        break;
      }
    }

    void PathDesigner::finalize_node_transformation(const EditorContext& context)
    {
      auto& transformation = *node_transformation_;
      node_transformation_ = boost::none;

      selected_path_->paths[selected_path_index_] = working_path_;
      context.editor_scene.commit_path(selected_path_, selected_path_index_);

      Vector2i world_offset;
            
      /*
      auto selected_layer = selected_layer_;
      auto path_index = selected_path_index_;

      switch (transformation.action)
      {
      case NodeTransformation::Append:
      {
        const auto& node = working_path_.nodes.back();

        auto action = [=]()
        {
          select_path(selected_layer, path_index, context.working_state);

          working_path_.nodes.push_back(node);
          commit_working_path(context.scene);
        };

        auto undo_action = [=]()
        {
          select_path(selected_layer, path_index, context.working_state);

          assert(!working_path_.nodes.empty());

          working_path_.nodes.pop_back();
          commit_working_path(context.scene);
        };

        context.action_history.push_action("Append node", action, undo_action, false);
        break;
      }

      case NodeTransformation::Move:
      {
        const auto& new_node = working_path_.nodes[transformation.id];

        auto action = [=]()
        {
          select_path(selected_layer, path_index, context.working_state);

          working_path_.nodes[transformation.id] = new_node;
          commit_working_path(context.scene);
        };

        auto undo_action = [=]()
        {
          select_path(selected_layer, path_index, context.working_state);

          assert(!working_path_.nodes.empty());

          working_path_.nodes[transformation.id] = transformation.original_state;
          commit_working_path(context.scene);
        };

        context.action_history.push_action("Edit node", action, undo_action, false);
        break;
      }

      default:
        break;
      }

      commit_working_path(context.scene);
      */
    }
  }
}