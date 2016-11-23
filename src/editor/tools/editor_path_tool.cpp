/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"
#include "editor_path_tool.hpp"

#include "editor/editor_context.hpp"
#include "editor/editor_state.hpp"

#include "imgui/imgui.h"

namespace ts
{
  namespace editor
  {
    namespace modes
    {
      enum PathToolModes
      {
        Nodes,
        Segments
      };

      const char* const mode_names[] =
      {
        "Node",
        "Segments"
      };
    }

    const char* PathTool::tool_name() const
    {
      return "Path Tool";
    }

    PathTool::mode_name_range PathTool::mode_names() const
    {
      return { std::begin(modes::mode_names), std::end(modes::mode_names) };
    }

    void PathTool::update_selection(const WorkingState& working_state)
    {
      auto selected_layer = working_state.selected_layer();
      if (selected_layer_ != selected_layer && selected_layer->type() == resources::TrackLayerType::Paths)
      {
        selected_layer_ = selected_layer;
        selected_path_index_ = 0;

        reload_working_path();
      }
    }

    void PathTool::ensure_path_layer_exists(const EditorContext& context)
    {
      // If we have a node transformation but no selected layer, create and select a layer.
      if (selected_layer_ == nullptr)
      {
        auto& track = context.scene.track();
        auto layer = track.create_layer("Path Layer", 0, resources::TrackLayerType::Paths);
        layer->paths().emplace_back();

        select_path(layer, 0, context.working_state);
      }

      // Otherwise, we might not have a path with the selected index. Need to fix that.
      else if (selected_path_index_ >= selected_layer_->paths().size())
      {
        auto index = selected_layer_->paths().size();
        selected_layer_->paths().emplace_back();

        select_path(selected_layer_, index, context.working_state);
      }
    }

    void PathTool::reload_working_path()
    {
      if (selected_layer_ && selected_path_index_ < selected_layer_->paths().size())
      {
        // Reload the working path.
        working_path_ = selected_layer_->paths()[selected_path_index_];
      }

      else
      {
        working_path_ = {};
      }
    }

    void PathTool::update_tool_info(const EditorContext& context)
    {
      update_selection(context.working_state);
    }

    void PathTool::update_canvas_interface(const EditorContext& context)
    {
      update_selection(context.working_state);

      auto mode = active_mode();
      if (mode == modes::Nodes)
      {
        /*** Features
         * Display current path and its nodes
         * Select nodes by clicking on them
         * Editing nodes by dragging them
         * Adding new nodes by clicking anywhere else
         ***/

        auto canvas_pos = ImGui::GetWindowPos();
        auto mouse_pos = ImGui::GetMousePos();
        auto canvas_mouse_pos = make_vector2<double>(mouse_pos.x - canvas_pos.x,
                                                     mouse_pos.y - canvas_pos.y);

        CoordTransform transformer(context);
        auto world_pos = vector2_cast<float>(transformer.world_position(canvas_mouse_pos));

        auto& nodes = working_path_.nodes;
        auto transform_pos = [&](Vector2f pos)
        {
          auto p = transformer.viewport_position(vector2_cast<double>(pos));

          return ImVec2(static_cast<float>(canvas_pos.x + p.x),
                        static_cast<float>(canvas_pos.y + p.y));
        };

        // Draw the path and its control points.
        // Also listen to events, so that we can modify the path.

        bool previously_hovered = false;

        auto draw_list = ImGui::GetWindowDrawList();
        for (const auto& node : nodes)
        {
          auto node_pos = transform_pos(node.position);
          auto first_pos = transform_pos(node.first_control);
          auto second_pos = transform_pos(node.second_control);

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
              auto node_id = &node - nodes.data();

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

        if (!node_transformation_)
        {
          if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered())
          {
            // Begin the addition of a new node.
            // Let's make sure a path layer exists first.
            ensure_path_layer_exists(context);

            Node node;
            node.first_control = node.position = node.second_control = world_pos;
            node.width = path_width_;
            nodes.push_back(node);

            NodeTransformation transformation{};
            transformation.original_state = nodes.back();
            transformation.action = NodeTransformation::Append;
            transformation.id = nodes.size() - 1;
            transformation.control = NodeTransformation::Base;

            node_transformation_.emplace(transformation);
          }
        }

        if (node_transformation_)
        {
          apply_node_transformation(world_pos);

          if (ImGui::IsMouseReleased(0))
          {
            finalize_node_transformation(context);

            node_transformation_ = boost::none;
          }
        }

        if (nodes.size() >= 2)
        {
          auto node_it = nodes.begin();
          auto node_end = nodes.end();
          auto next_it = std::next(node_it);
          for (; next_it != node_end; ++node_it, ++next_it)
          {
            auto p1 = transform_pos(node_it->position);
            auto p2 = transform_pos(node_it->second_control);
            auto p3 = transform_pos(next_it->first_control);
            auto p4 = transform_pos(next_it->position);

            std::uint32_t color = ImColor(0.5f, 1.0f, 1.0f, 1.0f);

            draw_list->AddBezierCurve(p1, p2, p3, p4, color, 2.0f);
          }
        }
      }

      else if (mode == modes::Segments)
      {

      }
    }

    void PathTool::apply_node_transformation(Vector2f final_position)
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

    void PathTool::finalize_node_transformation(const EditorContext& context)
    {
      auto& transformation = *node_transformation_;

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
      node_transformation_ = boost::none;
    }

    void PathTool::select_path(resources::TrackLayer* layer, std::size_t path_index, WorkingState& working_state)
    {
      working_state.select_layer(layer);

      if (layer && layer->type() != resources::TrackLayerType::Paths)
      {
        layer = nullptr;
        path_index = 0;
      }

      if (layer != selected_layer_ || selected_path_index_ != path_index)
      {
        selected_layer_ = layer;
        selected_path_index_ = path_index;

        reload_working_path();
      }
    }

    void PathTool::commit_working_path(const EditorScene& scene)
    {
      if (selected_layer_ && selected_path_index_)
      {
        // scene.commit_working_path(selected_layer_, selected_path_index_, working_path_);

        auto& paths = selected_layer_->paths();
        if (selected_path_index_ < paths.size()) paths[selected_path_index_] = working_path_;
      }
    }

    void PathTool::delete_last(const EditorContext& editor_context)
    {
      auto layer = selected_layer_;
      auto index = selected_path_index_;

      if (!working_path_.nodes.empty() && layer && index < layer->paths().size())
      {
        auto action = [=]()
        {
          select_path(layer, index, editor_context.working_state);
          working_path_.nodes.pop_back();
          commit_working_path(editor_context.scene);
        };

        const auto& node = working_path_.nodes.back();
        auto undo_action = [=]()
        {
          select_path(layer, index, editor_context.working_state);
          working_path_.nodes.push_back(node);
          commit_working_path(editor_context.scene);
        };

        editor_context.action_history.push_action("Remove node", action, undo_action);
      }
    }

    void PathTool::delete_selected(const EditorContext& context)
    {

    }
  }
}