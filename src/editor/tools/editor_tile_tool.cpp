/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "editor_tile_tool.hpp"

#include "editor/editor_scene.hpp"
#include "editor/editor_working_state.hpp"
#include "editor/editor_action_history.hpp"

#include "resources/track.hpp"
#include "resources/tile_library.hpp"
#include "resources/tile_expansion.hpp"

#include "scene/render_scene.hpp"
#include "scene/track_vertices.hpp"

#include "graphics/gl_check.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_guards.hpp"

namespace ts
{
  namespace editor
  {
    namespace modes
    {
      enum Mode
      {
        TilePlacement,
        TileSelection
      };
    }

    graphics::Texture load_tiled_transparency_texture()
    {
      sf::Image transparency_tiles;
      transparency_tiles.create(32, 32, sf::Color(160, 160, 160));
      for (unsigned y = 0; y != 16; ++y)      
        for (unsigned x = 0; x != 16; ++x)        
          transparency_tiles.setPixel(x, y, sf::Color(130, 130, 130));

      for (unsigned y = 16; y != 32; ++y)
        for (unsigned x = 16; x != 32; ++x)
          transparency_tiles.setPixel(x, y, sf::Color(130, 130, 130));

      return graphics::create_texture(transparency_tiles);
    }

    TileTool::TileTool()
      : tiled_transparency_texture_(load_tiled_transparency_texture())
    {
    }

    const char* TileTool::tool_name() const
    {
      return "Tile Tool";
    }

    void TileTool::update_tool_info(const EditorContext& context)
    {
      // Show tile library and update selected tile if needed

      const char* all[] =
      {
        "All Tiles"
      };      

      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(3, 3));
      ImGui::BeginChild("Tile Selection", ImVec2(ImGui::GetWindowSize().x, 200), true);      

      ImGui::PushItemWidth(ImGui::GetWindowSize().x - 5.0f);      
      ImGui::Combo("##tile_category_filter", &selected_tile_category_, all, static_cast<int>(std::end(all) - all));
      ImGui::PopItemWidth();

      if (auto render_scene = context.scene.render_scene())
      {
        // Draw tiles
        const auto& style = ImGui::GetStyle();

        auto selector_window_width = ImGui::GetWindowSize().x - style.WindowPadding.x;
        
        ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, ImColor(0.5f, 0.5f, 0.55f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImColor(0.5f, 0.6f, 0.8f, 0.3f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImColor(0.5f, 0.6f, 0.8f, 0.3f));
        ImGui::PushStyleColor(ImGuiCol_Header, ImColor(0, 0, 0, 0));

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

        ImGui::BeginChild("##tile_selector_list", ImVec2(selector_window_width, 160.f), true);        

        auto draw_list = ImGui::GetWindowDrawList();
        std::int32_t column = 0, max_columns = 3;
        
        auto next_quad_group = [](auto it, auto end, auto tile_id)
        {
          return std::find_if(it, end, [=](const TileQuad& q) { return q.tile_id != tile_id; });
        };

        std::uint32_t distinct_tile_count = 0;
        auto quad_begin = tile_library_quad_cache_.begin();
        auto quad_end = tile_library_quad_cache_.end();

        for (auto it = quad_begin; it != quad_end; ++distinct_tile_count)
        {
          it = next_quad_group(std::next(it), quad_end, it->tile_id);
        }

        ImVec2 cell_size(tile_library_cell_size_.x, tile_library_cell_size_.y);
        ImGuiListClipper clipper(distinct_tile_count / max_columns, cell_size.y);

        auto quad_it = tile_library_quad_cache_.begin();
        auto selected_id = placement_tile_id();
        
        for (int index = 0; index < clipper.DisplayStart * 3; ++index)
        {
          quad_it = next_quad_group(std::next(quad_it), quad_end, quad_it->tile_id);
        }

        if (selected_id && scroll_to_selected_)
        {
          std::uint32_t group_index = 0;
          for (auto it = quad_begin; it != quad_end && it->tile_id != *selected_id;)
          {
            it = next_quad_group(std::next(it), quad_end, it->tile_id);
            ++group_index;
          }

          auto offset = (group_index / max_columns) * cell_size.y;
          ImGui::SetScrollFromPosY(ImGui::GetCursorStartPos().y + offset, 0.0f);

          scroll_to_selected_ = false;
        }

        {
          auto window_pos = ImGui::GetWindowPos();
          auto window_size = ImGui::GetWindowSize();
          ImVec2 bottom_left(window_pos.x, window_pos.y + window_size.y);
          ImVec2 top_right(window_pos.x + window_size.x, window_pos.y);
          ImVec2 bottom_right(top_right.x, bottom_left.y);

          auto tex_id = reinterpret_cast<void*>(static_cast<std::uintptr_t>(tiled_transparency_texture_.get()));
          auto size_multiplier = 1.0f / tiled_transparency_texture_.size();

          ImVec2 tex_coords[] = { window_pos, bottom_left, bottom_right, top_right };
          for (auto& coords : tex_coords)
          {
            coords.x *= size_multiplier.x;
            coords.y *= size_multiplier.y;
          }
          
          draw_list->PushTextureID(tex_id);
          draw_list->PrimReserve(6, 4);
          draw_list->PrimQuadUV(window_pos, bottom_left, bottom_right, top_right,
                                tex_coords[0], tex_coords[1], tex_coords[2], tex_coords[3],
                                ImColor(1.0f, 1.0f, 1.0f, 1.0f)); 
          draw_list->PopTextureID();
        }       

        // Use this great little clipper tool to draw no more than we need
        // Significantly speeds up the rendering.
        while (clipper.Step())
        {
          for (auto idx = clipper.DisplayStart; idx < clipper.DisplayEnd; ++idx)
          {
            for (int col = 0; col < max_columns && quad_it != quad_end; ++col)
            {
              auto tile_id = quad_it->tile_id;
              bool selected = selected_id && tile_id == *selected_id;

              ImGui::PushID(tile_id);
              if (ImGui::Selectable("##tile_selector", selected, 0, cell_size))
              {
                set_placement_tile_id(tile_id, context.scene.track().tile_library());
              }

              ImGui::PopID();

              auto rect_min = ImGui::GetItemRectMin();
              auto rect_max = ImGui::GetItemRectMax();
              auto rect_size = make_vector2(rect_max.x - rect_min.x, rect_max.y - rect_min.y);

              auto vec2_cvt = [](Vector2f p)
              {
                return ImVec2(p.x, p.y);
              };

              auto transform = [=](Vector2f p)
              {
                return vec2_cvt(make_vector2(rect_min.x, rect_min.y) + rect_size * 0.5f + p);
              };

              auto range_end = next_quad_group(std::next(quad_it), quad_end, tile_id);
              for (; quad_it != range_end; ++quad_it)
              {
                const auto& quad_info = *quad_it;
                const auto& v = quad_info.vertices;

                draw_list->PushTextureID(quad_info.texture_handle);
                draw_list->PrimReserve(6, 4);
                draw_list->PrimQuadUV(transform(v[0].position), transform(v[1].position),
                                      transform(v[2].position), transform(v[3].position),

                                      vec2_cvt(v[0].texture_coords), vec2_cvt(v[1].texture_coords),
                                      vec2_cvt(v[2].texture_coords), vec2_cvt(v[3].texture_coords),

                                      ImColor(1.0f, 1.0f, 1.0f, 1.0f));
                draw_list->PopTextureID();
              }

              if (selected)
              {
                draw_list->AddRectFilled(rect_min, rect_max, ImColor(0.4f, 0.6f, 0.9f, 0.45f));
              }

              else
              {
                auto border_color = ImColor(0.25f, 0.25f, 0.25f, 1.0f);

                if (ImGui::IsItemHovered())
                {
                  border_color = ImColor(0.80f, 0.80f, 0.80f, 1.0f);
                }

                draw_list->AddRect(rect_min, rect_max, border_color, 0.0f, 0, 1.0f);
              }

              char buffer[32];
              std::sprintf(buffer, "%d", tile_id);

              ImVec2 text_pos(rect_min.x + 6.f, rect_max.y - 18.f);
              draw_list->AddText(text_pos, ImColor(1.0f, 1.0f, 1.0f, 0.7f), buffer);

              if (col + 1 != max_columns) ImGui::SameLine();
            }
          }
        }       

        ImGui::EndChild();        

        ImGui::PopStyleColor(4);
        ImGui::PopStyleVar(2);
      }

      ImGui::EndChild();
      ImGui::PopStyleVar(1);
    }

    void TileTool::update_canvas_interface(const EditorContext& context)
    {
      update_selected_layer(context);

      auto canvas_pos = ImGui::GetWindowPos();
      auto mouse_pos = ImGui::GetMousePos();
      auto canvas_mouse_pos = make_vector2<double>(mouse_pos.x - canvas_pos.x,
                                                   mouse_pos.y - canvas_pos.y);

      CoordTransform transformer(context);
      auto world_pos = transformer.world_position(canvas_mouse_pos);
      const auto& view_port = context.canvas_viewport;

      auto mode = active_mode();
      if (mode == modes::TilePlacement && selected_layer_)
      {
        auto& scene = context.scene;

        if (context.canvas_focus && ImGui::IsMouseClicked(0))
        {
          // Place a tile
          place_tile_at(context, world_pos);
        }

        update_placement_tile_preview(scene, world_pos);
      }
    }

    void TileTool::on_canvas_render(const ImmutableEditorContext& context, const sf::Transform& view_matrix) const
    {
      if (context.working_state.selected_layer() != nullptr)
      {
        tile_interaction_renderer_.render(view_matrix);
      }      
    }

    void TileTool::next(const EditorContext& context)
    {
      const auto& tile_lib = context.scene.track().tile_library();

      auto advance = [&](std::int32_t next_tile_id)
      {
        const auto& tiles = tile_lib.tiles();
        const auto& tile_groups = tile_lib.tile_groups();

        auto next_tile = tiles.lower_bound(next_tile_id);
        auto next_group = tile_groups.lower_bound(next_tile_id);

        placement_tile_ = nullptr;
        placement_tile_group_ = nullptr;

        // Find the thing with the lowest id, tile or tile group.        
        if (next_tile != tiles.end() && next_group != tile_groups.end())
        {
          if (next_tile->id < next_group->id) placement_tile_ = &*next_tile;
          else placement_tile_group_ = &*next_group;
        }

        else if (next_tile != tiles.end()) placement_tile_ = &*next_tile;
        else if (next_group != tile_groups.end()) placement_tile_group_ = &*next_group;
      };

      auto old_tile = placement_tile_;
      auto old_group = placement_tile_group_;

      if (placement_tile_)
      {
        advance(placement_tile_->id + 1);
      }

      else if (placement_tile_group_)
      {
        advance(placement_tile_group_->id + 1);
      }

      if (!placement_tile_ && !placement_tile_group_)
      {
        advance(0);
      }

      if ((placement_tile_ && placement_tile_ != old_tile) ||
          (old_tile != placement_tile_ || old_group != placement_tile_group_))
      {
        placement_tile_dirty_ = true;
        scroll_to_selected_ = true;
      }
    }
    
    void TileTool::previous(const EditorContext& context)
    {

    }

    boost::optional<std::uint32_t> TileTool::placement_tile_id() const
    {
      if (!placement_tile_ && !placement_tile_group_) return boost::none;
      if (placement_tile_) return placement_tile_->id;
      return placement_tile_group_->id;
    }

    void TileTool::set_placement_tile_id(std::uint32_t tile_id, const resources::TileLibrary& tile_lib)
    {
      const auto& tiles = tile_lib.tiles();

      // First attempt to find a regular tile with the given id
      auto tile_it = tiles.find(tile_id);
      if (tile_it != tiles.end())
      {
        auto new_val = &*tile_it;
        placement_tile_dirty_ = (new_val != placement_tile_);
        placement_tile_ = new_val;        
      }

      else
      {
        // Then, try to find a matching tile group.
        const auto& groups = tile_lib.tile_groups();

        auto group_it = groups.find(tile_id);
        if (group_it != groups.end())
        {
          auto new_val = &*group_it;
          if (placement_tile_ || new_val != placement_tile_group_)
          {
            placement_tile_ = nullptr;
            placement_tile_group_ = new_val;
            placement_tile_dirty_ = true;
          }
        }
      }
    }

    void TileTool::place_tile_at(const EditorContext& context, Vector2d world_pos)
    {
      if (auto tile_id = placement_tile_id())
      {
        resources::Tile tile;
        tile.position = vector2_cast<std::int32_t>(world_pos);
        tile.id = *tile_id;
        tile.level = 0;
        tile.rotation = placement_tile_rotation_;

        auto layer = selected_layer_;
        if (auto tiles = layer->tiles())
        {
          auto tile_index = static_cast<std::uint32_t>(tiles->size());

          auto action = [=]()
          {
            select_layer(layer, context);
            context.scene.append_tile(layer, tile);
          };

          auto undo_action = [=]()
          {
            select_layer(layer, context);
            context.scene.remove_tile(layer, tile_index);
          };

          context.action_history.push_action("Place tile", action, undo_action);
        }
      }      
    }
    
    void TileTool::update_placement_tile_preview(EditorScene& editor_scene, Vector2d world_pos)
    {
      auto* render_scene = editor_scene.render_scene();

      if (placement_tile_dirty_ && render_scene)
      {
        const auto& texture_mapping = editor_scene.render_scene()->track_scene().texture_mapping();
        const auto& tile_library = editor_scene.track().tile_library();

        if (placement_tile_)
        {
          resources::PlacedTile dummy;
          dummy.id = placement_tile_->id;
          dummy.definition = placement_tile_;
          dummy.rotation = placement_tile_rotation_;
          tile_interaction_renderer_.update_tile_geometry(&dummy, 1, texture_mapping);
        }

        else if (placement_tile_group_)
        {
          const auto& sub_tiles = placement_tile_group_->sub_tiles;
          auto& cache = tile_expansion_cache_;

          cache.resize(sub_tiles.size());
          std::transform(sub_tiles.begin(), sub_tiles.end(), cache.begin(),
                         [&](const resources::Tile& tile)
          {
            resources::PlacedTile result;
            result.id = tile.id;
            result.level = tile.level;
            result.position = tile.position;
            result.rotation = tile.rotation;
            result.definition = nullptr;

            const auto& tiles = tile_library.tiles();
            auto tile_it = tiles.find(tile.id);
            if (tile_it != tiles.end()) result.definition = &*tile_it;

            return result;
          });

          tile_interaction_renderer_.update_tile_geometry(cache.data(), cache.size(), texture_mapping);
        }

        else
        {
          tile_interaction_renderer_.clear_tile_geometry();
        }

        placement_tile_dirty_ = false;
      }

      auto rad = degrees(static_cast<float>(placement_tile_rotation_)).radians();

      auto model_matrix = sf::Transform().translate(world_pos.x, world_pos.y).rotate(rad);
      tile_interaction_renderer_.set_transform(model_matrix);
    }

    TileTool::mode_name_range TileTool::mode_names() const
    {
      static const char* const names[] =
      {
        "Placement"
      };

      return mode_name_range(std::begin(names), std::end(names));
    }

    void TileTool::select_layer(resources::TrackLayer* layer, const EditorContext& context)
    {
      context.working_state.select_layer(layer);
      update_selected_layer(context);
    }

    void TileTool::update_selected_layer(const EditorContext& context)
    {
      auto selected_layer = context.working_state.selected_layer();
      if (selected_layer != selected_layer_)
      {
        selected_layer_ = selected_layer;

       // Deselect everything
      }
    }

    void TileTool::delete_selected(const EditorContext& context)
    {
      update_selected_layer(context);
    }

    void TileTool::delete_last(const EditorContext& context)
    {
      update_selected_layer(context);

      if (auto tiles = selected_layer_->tiles())
      {
        auto layer = selected_layer_;
        if (!tiles->empty())
        {
          auto last_tile = tiles->back();

          auto action = [=]()
          {
            select_layer(layer, context);

            context.scene.remove_last_tile(layer);
          };

          auto undo_action = [=]()
          {
            select_layer(layer, context);

            context.scene.append_tile(layer, last_tile);
          };

          context.action_history.push_action("Remove tile", action, undo_action);
        }
      }
    }

    void TileTool::activate(const EditorContext& context)
    {
      if (!placement_tile_ && !placement_tile_group_)
      {
        next(context);

        reload_tile_library_cache(context.scene);
      }
    }

    void TileTool::deactivate(const EditorContext& context)
    {

    }

    void TileTool::reload_tile_library_cache(const EditorScene& editor_scene)
    {
      tile_library_quad_cache_.clear();
      if (auto render_scene = editor_scene.render_scene())
      {
        const auto& tile_library = editor_scene.track().tile_library();
        const auto& track_scene = render_scene->track_scene();
        const auto& texture_mapping = track_scene.texture_mapping();

        auto calculate_scale_factor = [](const resources::TileDefinition& tile_def)
        {
          return vector2_cast<float>(size(tile_def.pattern_rect)) / size(tile_def.image_rect);
        };

        auto calculate_fitting_scale = [=](const resources::PlacedTile& tile)
        {
          const auto& image_rect = tile.definition->image_rect;
          const auto& pattern_rect = tile.definition->pattern_rect;

          auto cell_size = tile_library_cell_size_;
          auto scale_factor = calculate_scale_factor(*tile.definition);

          return scale_factor * std::min({ cell_size.x / image_rect.width, cell_size.y / image_rect.height, 1.0f });
        };

        std::vector<TileQuad> result;
        result.reserve(1024);

        auto add_quad = [&](std::uint32_t tile_id, const graphics::Texture* texture_handle, const auto& vertices)
        {
          TileQuad quad;
          quad.tile_id = tile_id;
          quad.texture_handle = reinterpret_cast<void*>(static_cast<std::uintptr_t>(texture_handle->get()));
          quad.vertices[0] = vertices[0];
          quad.vertices[1] = vertices[1];

          // generate_tile_vertices generates a different vertex pattern than ImGui expects, 
          // so we need to switch up the order a bit.
          quad.vertices[2] = vertices[3];          
          quad.vertices[3] = vertices[2]; 

          result.push_back(quad);
        };

        auto fit_into_cell = [&](auto quad_it, auto quad_end)
        {
          if (quad_it != quad_end)
          {
            using V = resources::Vertex;
            auto x_cmp = [](const V& a, const V& b)
            {
              return a.position.x < b.position.x;
            };

            auto y_cmp = [](const V& a, const V& b)
            {
              return a.position.y < b.position.y;
            };

            // Find the bounding box of all quads in the given range.
            auto minmax_x = std::minmax_element(quad_it->vertices.begin(), quad_it->vertices.end(), x_cmp);
            auto minmax_y = std::minmax_element(quad_it->vertices.begin(), quad_it->vertices.end(), y_cmp);

            auto min_corner = make_vector2(minmax_x.first->position.x, minmax_y.first->position.y);
            auto max_corner = make_vector2(minmax_x.second->position.x, minmax_y.second->position.y);

            for (auto it = std::next(quad_it); it != quad_end; ++it)
            {
              auto x = std::minmax_element(it->vertices.begin(), it->vertices.end(), x_cmp);
              auto y = std::minmax_element(it->vertices.begin(), it->vertices.end(), y_cmp);

              auto min_x = x.first->position.x, max_x = x.second->position.x;
              auto min_y = y.first->position.y, max_y = y.second->position.y;
              
              if (min_x < min_corner.x) min_corner.x = min_x;
              if (min_y < min_corner.y) min_corner.y = min_y;

              if (max_x > max_corner.x) max_corner.x = max_x;
              if (max_y > max_corner.y) max_corner.y = max_y;
            }

            auto box_size = max_corner - min_corner;
            auto center = min_corner + box_size * 0.5f;

            auto cell_size = tile_library_cell_size_ - 5;

            auto scale = cell_size / box_size;
            auto scale_factor = std::min({ scale.x, scale.y, 2.0f });

            // And transform all quad vertices such that they fit into the cell.
            for (auto it = quad_it; it != quad_end; ++it)
            {
              for (V& vertex : it->vertices)
              {
                vertex.position -= center;
                vertex.position *= scale_factor;                
              }
            }
          }
        };

        // For all tiles in the library
        for (const auto& tile : tile_library.tiles())
        {
          auto image_rect = rect_cast<float>(tile.image_rect);

          resources::PlacedTile dummy_tile = {};
          dummy_tile.definition = &tile;
          dummy_tile.id = tile.id;

          auto quad_index = result.size();

          auto range = texture_mapping.find(texture_mapping.tile_id(tile.id));
          for (const auto& mapping : range)
          {
            auto vertices = scene::generate_tile_vertices(dummy_tile, tile,
                                                          mapping.texture_rect, mapping.fragment_offset,
                                                          1.0f / mapping.texture->size());

            add_quad(tile.id, mapping.texture, vertices);
          }

          fit_into_cell(result.begin() + quad_index, result.end());
        }

        std::size_t tile_group_start = result.size();
        std::vector<resources::PlacedTile> sub_tile_cache;
        sub_tile_cache.reserve(128);

        for (const auto& tile_group : tile_library.tile_groups())
        {
          sub_tile_cache.clear();
          resources::expand_tiles(tile_group.sub_tiles.begin(), tile_group.sub_tiles.end(), tile_library,
                                  std::back_inserter(sub_tile_cache));

          auto quad_index = result.size();
          for (auto sub_tile : sub_tile_cache)
          {
            auto range = texture_mapping.find(texture_mapping.tile_id(sub_tile.id));
            for (const auto& mapping : range)
            {
              auto vertices = scene::generate_tile_vertices(sub_tile, *sub_tile.definition,
                                                            mapping.texture_rect, mapping.fragment_offset,
                                                            1.0f / mapping.texture->size());

              add_quad(tile_group.id, mapping.texture, vertices);
            }
          }

          fit_into_cell(result.begin() + quad_index, result.end());
        }

        // Merge both ranges, tiles and tile groups, into the output vector.
        tile_library_quad_cache_.resize(result.size());
        std::merge(result.begin(), result.begin() + tile_group_start,
                   result.begin() + tile_group_start, result.end(),
                   tile_library_quad_cache_.begin(),
                   [](const TileQuad& a, const TileQuad& b)
        {
          return a.tile_id < b.tile_id;
        });
      }
    }
  }
}