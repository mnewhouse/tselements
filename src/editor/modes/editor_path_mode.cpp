/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "editor_path_mode.hpp"

#include "editor/editor_context.hpp"
#include "editor/editor_state.hpp"

#include "utility/interpolate.hpp"

#include "imgui/imgui.h"

namespace ts
{
  namespace editor
  {
    namespace tools
    {
      enum PathModeTools
      {
        Nodes,
        Segments
      };

      const char* const tool_names[] =
      {
        "Node",
        "Segments"
      };
    }

    const char* PathMode::mode_name() const
    {
      return "Paths";
    }

    PathMode::tool_name_range PathMode::tool_names() const
    {
      return { std::begin(tools::tool_names), std::end(tools::tool_names) };
    }

    void PathMode::update_selection(WorkingState& working_state)
    {
      auto layer = working_state.selected_layer();
      if (layer != selected_layer_)
      {
        selected_layer_ = layer;
        if (auto style = selected_layer_->path_style())
        {
          working_state.select_path(style->path);
        }
      }

      auto path = working_state.selected_path();
      if (path != selected_path_)
      {
        selected_path_ = path;
        reload_working_path();
      }
    }

    void PathMode::ensure_path_exists(const EditorContext& context)
    {
      // If we have a node transformation but no selected layer, create and select a layer.
      if (selected_path_ == nullptr)
      {
        auto& track = context.scene.track();
        auto path = track.path_library().create_path();

        auto kerb_layer = track.create_layer(resources::TrackLayerType::PathStyle, "Kerb", 0);
        auto kerb_style = kerb_layer->path_style();
        kerb_style->path = path;
        kerb_style->style.border_only = true;
        kerb_style->style.fade_length = 24.0f;
        kerb_style->style.terrain_id = 15;
        kerb_style->style.border_texture = 16;
        kerb_style->style.width = 64.0f;
        kerb_style->style.border_width = 5.0f;
        kerb_style->style.is_segmented = true;
        kerb_style->style.texture_mode = resources::PathStyle::Directional;

        auto layer = track.create_layer(resources::TrackLayerType::PathStyle, "Road", 0);
        auto style = layer->path_style();
        style->path = path;
        style->style.base_texture = 2;
        style->style.border_texture = 15;
        style->style.terrain_id = 2;
        style->style.border_width = 1.5f;
        style->style.width = 64.0f;

        select_path(path, context.working_state);
      }
    }

    void PathMode::reload_working_path()
    {
      if (selected_path_ && selected_sub_path_index_ < selected_path_->sub_paths.size())
      {
        // Reload the working path.
        working_path_ = selected_path_->sub_paths[selected_sub_path_index_];
      }

      else
      {
        working_path_ = {};
      }
    }

    void PathMode::update_tool_info(const EditorContext& context)
    {
      update_selection(context.working_state);
    }

    void PathMode::update_canvas_interface(const EditorContext& context)
    {
      update_selection(context.working_state);

      auto canvas_pos = ImGui::GetWindowPos();
      auto mouse_pos = ImGui::GetMousePos();
      auto canvas_mouse_pos = make_vector2<double>(mouse_pos.x - canvas_pos.x,
                                                   mouse_pos.y - canvas_pos.y);

      CoordTransform transformer(context);
      auto world_pos = vector2_cast<float>(transformer.world_position(canvas_mouse_pos));

      auto transform_pos = [&](Vector2f pos)
      {
        auto p = transformer.viewport_position(vector2_cast<double>(pos));

        return ImVec2(static_cast<float>(canvas_pos.x + p.x),
                      static_cast<float>(canvas_pos.y + p.y));
      };

      auto draw_list = ImGui::GetWindowDrawList();

      auto tool = active_tool();
      if (tool == tools::Nodes)
      {
        /*** Features
         * Display current path and its nodes
         * Select nodes by clicking on them
         * Editing nodes by dragging them
         * Adding new nodes by clicking anywhere else
         ***/

        auto& nodes = working_path_.nodes;


        // Draw the path and its control points.
        // Also listen to events, so that we can modify the path.

        bool previously_hovered = false;

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
              auto node_id = static_cast<std::uint32_t>(&node - nodes.data());

              NodeTransformation transformation;
              transformation.control = control;
              transformation.id = node_id;
              transformation.action = NodeTransformation::Initiate;
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
          if (context.canvas_focus && ImGui::IsMouseClicked(0))
          {
            // Begin the addition of a new node.
            // Let's make sure a path layer exists first.
            ensure_path_exists(context);

            if (!working_path_.closed)
            {
              Node node;
              node.first_control = node.position = node.second_control = world_pos;
              node.width = 0.0f;
              nodes.push_back(node);

              NodeTransformation transformation{};
              transformation.original_state = nodes.back();
              transformation.action = NodeTransformation::Append;
              transformation.id = static_cast<std::uint32_t>(nodes.size() - 1);
              transformation.control = NodeTransformation::Base;

              node_transformation_.emplace(transformation);
            }
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

          std::uint32_t color = ImColor(0.5f, 1.0f, 1.0f, 1.0f);
          for (; next_it != node_end; ++node_it, ++next_it)
          {
            auto p1 = transform_pos(node_it->position);
            auto p2 = transform_pos(node_it->second_control);
            auto p3 = transform_pos(next_it->first_control);
            auto p4 = transform_pos(next_it->position);

            draw_list->AddBezierCurve(p1, p2, p3, p4, color, 2.0f);
          }

          if (working_path_.closed)
          {
            auto p1 = transform_pos(nodes.back().position);
            auto p2 = transform_pos(nodes.back().second_control);
            auto p3 = transform_pos(nodes.front().first_control);
            auto p4 = transform_pos(nodes.front().position);            

            draw_list->AddBezierCurve(p1, p2, p3, p4, color, 2.0f);
          }
        }
      }

      else if (tool == tools::Segments && selected_layer_)
      {
        auto path = selected_path_;
        auto layer = selected_layer_;
        auto style = selected_layer_->path_style();
        if (style && style->style.is_segmented)
        {
          if (path_outline_cache_.empty())
          {
            rebuild_path_outline_cache(*style);
          }

          // Draw existing segments for selected layer
          // Segments have 2 handles which lets the user drag them around

          // If no currently active segment transformation, find the closest matching outline point and show
          // a handle there, if a match was found.
          {
            using resources::StrokeSegment;
            auto best_distance = 20.0f;
            auto best_point_it = path_outline_cache_.end();
            auto best_time_point = 0.0f;
            auto best_side = StrokeSegment::First;
            auto best_sub_path_id = std::size_t{};

            auto it = path_outline_partitions_.begin(), next_it = std::next(it);
            for (; next_it != path_outline_partitions_.end(); ++it, ++next_it)
            {
              auto side = it->invert_normal ? resources::StrokeSegment::Second : resources::StrokeSegment::First;

              if (it->point_id == next_it->point_id) continue;
              if (segment_transformation_ && segment_transformation_->side != side) continue;

              auto point_it = path_outline_cache_.begin() + it->point_id, next_point_it = std::next(point_it);
              auto point_end = path_outline_cache_.begin() + next_it->point_id;
              for (; next_point_it != point_end; ++point_it, ++next_point_it)
              {
                auto a = world_pos - point_it->point;
                auto b = next_point_it->point - point_it->point;

                auto len_sq = magnitude_squared(b);

                auto d = dot_product(a, b);
                if (len_sq >= 0.0001f && d >= 0.0 && d <= len_sq)
                {
                  auto c = std::abs(cross_product(a, b));
                  auto len = magnitude(b);
                  auto inv_len = 1.0f / len;
                  auto dist = c * inv_len;
                  if (dist < best_distance)
                  {
                    best_side = side;
                    best_time_point = d / len_sq;
                    best_distance = dist;
                    best_sub_path_id = it->sub_path_id;
                    best_point_it = point_it;
                  }
                }
              }
            }

            auto handle_radius = 5.0f;
            auto handle_color = ImColor(1.0f, 0.6f, 0.0f);
            auto handle_hover_color = ImColor(1.0f, 1.0f, 0.8f);

            auto handle_tl = [=](auto vec)
            {
              return ImVec2(vec.x - handle_radius, vec.y - handle_radius);
            };

            auto handle_br = [=](auto vec)
            {
              return ImVec2(vec.x + handle_radius, vec.y + handle_radius);
            };

            auto handle_contains = [=](auto vec, Vector2f point)
            {
              return contains(FloatRect(vec.x - handle_radius, vec.y - handle_radius, handle_radius * 2.0f, handle_radius * 2.0f),
                              point);
            };

            bool hover_state = false;
            if (segment_transformation_)
            {
              auto& st = *segment_transformation_;
              if (st.action == st.Create)
              {
                auto p = transform_pos(st.position);
                draw_list->AddRectFilled(handle_tl(p), handle_br(p), handle_color);
              }

              else if (st.action == st.Drag)
              { 
                auto& seg = style->style.segments[st.segment_idx];

                auto offset = style->style.width * 0.5f;
                if (st.side != seg.First) offset = -offset;

                auto point = st.point_idx == 0 ? seg.start_time_point : seg.end_time_point;
                auto other = st.point_idx == 0 ? seg.end_time_point : seg.start_time_point;
                
                auto& sub_path = style->path->sub_paths[seg.sub_path_id];
                auto p2 = transform_pos(path_point_at(sub_path, other, offset));

                draw_list->AddRectFilled(handle_tl(p2), handle_br(p2), handle_color);

                if (best_point_it != path_outline_cache_.end())
                {
                  auto p1 = transform_pos(interpolate_linearly(best_point_it->point, best_point_it[1].point, best_time_point));
                  draw_list->AddRectFilled(handle_tl(p1), handle_br(p1), handle_color);

                  if (!ImGui::IsMouseDown(0))
                  {
                    auto t = interpolate_linearly(best_point_it->time_point, best_point_it[1].time_point, best_time_point);
                    auto new_seg = seg;
                    if (st.point_idx == 0) new_seg.start_time_point = t;
                    else new_seg.end_time_point = t;

                    auto segment_idx = st.segment_idx;
                    auto action = [=]()
                    {
                      selected_stroke_segment_index_ = segment_idx;
                      style->style.segments[segment_idx] = new_seg;
                      context.scene.rebuild_path_layer(layer);
                    };

                    auto undo_action = [=]()
                    {
                      selected_stroke_segment_index_ = segment_idx;
                      style->style.segments[segment_idx] = seg;
                      context.scene.rebuild_path_layer(layer);
                    };

                    context.action_history.push_action("Edit stroke segment", action, undo_action);
                    segment_transformation_ = boost::none;
                  }
                }
              }
            }

            else
            {
              //auto existing_handle_color = ImColor(1.0f, 0.6f, 0.0f);
              std::uint32_t segment_idx = 0;
              for (auto& seg : style->style.segments)
              {
                auto offset = style->style.width * 0.5f;
                if (seg.side != seg.First) offset = -offset;

                auto& sub_path = style->path->sub_paths[seg.sub_path_id];
                auto p1 = transform_pos(path_point_at(sub_path, seg.start_time_point, offset));
                auto p2 = transform_pos(path_point_at(sub_path, seg.end_time_point, offset));

                auto p1_hover = !hover_state && handle_contains(p1, { mouse_pos.x, mouse_pos.y });
                if (p1_hover) hover_state = true;

                auto p2_hover = !hover_state && handle_contains(p2, { mouse_pos.x, mouse_pos.y });
                if (p2_hover) hover_state = true;

                auto selected = seg.sub_path_id == selected_sub_path_index_ &&
                  segment_idx == selected_stroke_segment_index_;

                draw_list->AddRectFilled(handle_tl(p1), handle_br(p1), p1_hover || selected ? handle_hover_color : handle_color);
                draw_list->AddRectFilled(handle_tl(p2), handle_br(p2), p2_hover || selected ? handle_hover_color : handle_color);

                if ((p1_hover || p2_hover) && ImGui::IsMouseDown(0))
                {
                  SegmentTransformation st{};
                  st.action = st.Drag;
                  st.sub_path_id = seg.sub_path_id;
                  st.side = seg.side;
                  st.segment_idx = segment_idx;
                  st.point_idx = p1_hover ? 0 : 1;
                  segment_transformation_.emplace(st);
                }

                ++segment_idx;
              }
            }

            if (!hover_state && best_point_it != path_outline_cache_.end())
            {
              auto next = std::next(best_point_it);
              auto point = next->point * best_time_point + best_point_it->point * (1.0f - best_time_point);

              auto handle_radius = 5.0f;

              auto transformed = transform_pos(point);
              draw_list->AddRectFilled(handle_tl(transformed), handle_br(transformed), handle_color);

              auto real_time_point = interpolate_linearly(best_point_it->time_point, next->time_point, best_time_point);

              if (ImGui::IsMouseClicked(0))
              {
                if (!segment_transformation_)
                {
                  SegmentTransformation st;
                  st.sub_path_id = best_sub_path_id;
                  st.time_point = real_time_point;
                  st.action = st.Create;
                  st.side = best_side;
                  st.position = interpolate_linearly(best_point_it->point, next->point, best_time_point);
                  segment_transformation_.emplace(st);
                }

                else if (segment_transformation_->action == SegmentTransformation::Create)
                {
                  resources::StrokeSegment seg;
                  seg.start_time_point = segment_transformation_->time_point;
                  seg.end_time_point = real_time_point;
                  seg.side = segment_transformation_->side;
                  seg.sub_path_id = segment_transformation_->sub_path_id;                  

                  auto& sub_path = path->sub_paths[seg.sub_path_id];
                  if (seg.end_time_point < seg.start_time_point)
                  {
                    std::swap(seg.start_time_point, seg.end_time_point);
                  }
                  
                  if (seg.end_time_point - seg.start_time_point > sub_path.nodes.size() * 0.5f)
                  {
                    std::swap(seg.end_time_point, seg.start_time_point);
                  }

                  auto action = [=]()
                  {
                    selected_stroke_segment_index_ = style->style.segments.size();
                    selected_sub_path_index_ = seg.sub_path_id;
                    style->style.segments.push_back(seg);
                    context.scene.rebuild_path_layer(layer);                    
                  };

                  auto undo_action = [=]()
                  {
                    style->style.segments.pop_back();
                    context.scene.rebuild_path_layer(layer);
                  };

                  context.action_history.push_action("Add stroke segment", action, undo_action);
                  segment_transformation_ = boost::none;
                }
              }
            }
          }
        }
      }
    }

    void PathMode::close_working_path(const EditorContext& context)
    {
      auto path = selected_path_;

      auto action = [=]()
      {
        select_path(path, context.working_state);

        working_path_.closed = true;
        commit_working_path(context.scene);
      };      

      auto undo_action = [=]()
      {
        select_path(path, context.working_state);

        working_path_.closed = false;
        commit_working_path(context.scene);
      };

      context.action_history.push_action("Close path", action, undo_action);
    }

    void PathMode::apply_node_transformation(Vector2f final_position)
    {
      auto& nodes = working_path_.nodes;

      if (node_transformation_->action == NodeTransformation::Initiate)
      {
        auto offset = final_position - node_transformation_->original_state.position;
        if (magnitude_squared(offset) >= 0.25f)
        {
          node_transformation_->action = NodeTransformation::Move;
        }        
      }

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

    void PathMode::finalize_node_transformation(const EditorContext& context)
    {
      auto& transformation = *node_transformation_;

      auto path = selected_path_;

      switch (transformation.action)
      {
      case NodeTransformation::Initiate:
        if (transformation.id == 0)
        {
          close_working_path(context);
        }
        break;
      case NodeTransformation::Append:
      {
        const auto& node = working_path_.nodes.back();

        auto action = [=]()
        {
          select_path(path, context.working_state);

          working_path_.nodes.push_back(node);
          commit_working_path(context.scene);
        };

        auto undo_action = [=]()
        {
          select_path(path, context.working_state);

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
          select_path(path, context.working_state);

          working_path_.nodes[transformation.id] = new_node;
          commit_working_path(context.scene);
        };

        auto undo_action = [=]()
        {
          select_path(path, context.working_state);

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

    void PathMode::select_path(resources::TrackPath* path, WorkingState& working_state)
    {
      working_state.select_path(path);

      if (path != selected_path_)
      {
        selected_path_ = path;        

        reload_working_path();
      }
    }

    void PathMode::commit_working_path(EditorScene& scene)
    {
      if (selected_path_)
      {   
        auto& sub_paths = selected_path_->sub_paths;
        if (selected_sub_path_index_ >= sub_paths.size())
        {
          sub_paths.resize(selected_sub_path_index_ + 1);
        }

        sub_paths[selected_sub_path_index_] = working_path_;

        scene.rebuild_path_geometry(selected_path_);
      }
    }

    void PathMode::delete_last(const EditorContext& editor_context)
    {
      auto path = selected_path_;
      if (active_tool() == tools::Nodes)
      {
        if (!working_path_.nodes.empty() && path)
        {
          if (!working_path_.closed)
          {
            auto action = [=]()
            {
              select_path(path, editor_context.working_state);
              working_path_.nodes.pop_back();
              commit_working_path(editor_context.scene);
            };

            const auto& node = working_path_.nodes.back();
            auto undo_action = [=]()
            {
              select_path(path, editor_context.working_state);
              working_path_.nodes.push_back(node);
              commit_working_path(editor_context.scene);
            };

            editor_context.action_history.push_action("Remove node", action, undo_action);
          }

          else
          {
            auto action = [=]()
            {
              select_path(path, editor_context.working_state);
              working_path_.closed = false;
              commit_working_path(editor_context.scene);
            };

            auto undo_action = [=]()
            {
              select_path(path, editor_context.working_state);
              working_path_.closed = true;
              commit_working_path(editor_context.scene);
            };

            editor_context.action_history.push_action("Unclose path", action, undo_action);
          }
        }
      }
    }

    void PathMode::delete_selected(const EditorContext& context)
    {
      if (active_tool() == tools::Segments && selected_layer_)
      {
        auto layer = selected_layer_;
        if (auto style = layer->path_style())
        {
          auto segment_index = selected_stroke_segment_index_;
          if (segment_index < style->style.segments.size())
          {            
            auto& seg = style->style.segments[segment_index];
            auto action = [=]()
            {
              selected_stroke_segment_index_ = -1;
              style->style.segments.erase(style->style.segments.begin() + segment_index);
              context.scene.rebuild_path_layer(layer);
            };

            auto undo_action = [=]()
            {
              auto& s = style->style.segments;
              s.insert(s.begin() + segment_index, seg);
              selected_stroke_segment_index_ = segment_index;              
              context.scene.rebuild_path_layer(layer);
            };

            context.action_history.push_action("Delete stroke segment", action, undo_action);
          }         
        }
      }
    }

    void PathMode::rebuild_path_outline_cache(const resources::PathLayerData& p)
    {
      path_outline_cache_.clear();
      path_outline_partitions_.clear();

      path_outline_width_ = static_cast<std::uint32_t>(p.style.width);
      std::size_t sub_path_id = 0;
      for (const auto& sub_path : p.path->sub_paths)
      {
        scene::OutlineProperties props;
        props.width = p.style.width;
        props.tolerance = 1.0f;        

        path_outline_partitions_.push_back({ sub_path_id, path_outline_cache_.size(), false });
        scene::generate_path_segment_outline(sub_path, props, path_outline_cache_);

        path_outline_partitions_.push_back({ sub_path_id, path_outline_cache_.size(), true });
        scene::generate_path_segment_outline(sub_path, invert(props), path_outline_cache_);

        ++sub_path_id;
      }

      // Add sentinel thing
      path_outline_partitions_.push_back({ sub_path_id, path_outline_cache_.size() });
    }
  }
}