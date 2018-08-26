/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "editor_scene.hpp"

#include "resources/tile_expansion.hpp"

#include "scene/track_scene_generator.hpp"
#include "scene/track_vertices.hpp"

#include <utility/vector3.hpp>

#include <array>

namespace ts
{
  namespace editor
  {
    const Colorf editor_bg_color = { 0.5f, 0.5f, 0.5f, 1.0f };

    EditorScene::EditorScene(resources::Track track)
      : track_(std::move(track)),
      render_scene_(scene::generate_track_scene(track_, true))
    {
      render_scene_->set_background_color(editor_bg_color);
    }

    const resources::Track& EditorScene::track() const
    {
      return track_;
    }

    resources::Track& EditorScene::track()
    {
      return track_;
    }

    void EditorScene::render(const scene::Viewport& view_port, Vector2i screen_size, double frame_progress,
                             const scene::RenderScene::render_callback& post_render) const
    {
      if (render_scene_)
      {
        render_scene_->render(view_port, screen_size, frame_progress, post_render);
      }      
    }

    scene::RenderScene EditorScene::steal_render_scene()
    {
      auto render_scene = std::move(*render_scene_);
      render_scene_ = boost::none;

      return render_scene;
    }

    void EditorScene::adopt_render_scene(scene::RenderScene render_scene)
    {

      render_scene_ = boost::none;
      render_scene_.emplace(std::move(render_scene));

      render_scene_->set_background_color(editor_bg_color);
      render_scene_->clear_dynamic_state();
    }

    const scene::RenderScene* EditorScene::render_scene() const
    {
      return render_scene_.get_ptr();
    }

    void EditorScene::deactivate_layer(resources::TrackLayer* layer)
    {
      track_.deactivate_layer(layer);
      render_scene_->remove_layer(layer);
    }

    void EditorScene::activate_layer(resources::TrackLayer* layer)
    {
      track_.activate_layer(layer);
      render_scene_->add_layer(layer);
    }      

    void EditorScene::show_layer(resources::TrackLayer* layer)
    {
      layer->set_visible(true);
      render_scene_->update_layer_visibility(layer);
    }

    void EditorScene::hide_layer(resources::TrackLayer* layer)
    {
      layer->set_visible(false);
      render_scene_->update_layer_visibility(layer);
    }

    const std::vector<resources::PlacedTile>& EditorScene::expand_tile(const resources::Tile& tile) const
    {
      tile_expansion_cache_.clear();
      resources::expand_tiles(&tile, &tile + 1, track_.tile_library(), std::back_inserter(tile_expansion_cache_));
      return tile_expansion_cache_;
    }

    template <typename TileIt>
    const std::vector<resources::PlacedTile>& EditorScene::expand_tiles(TileIt it, TileIt end) const
    {
      tile_expansion_cache_.clear();
      resources::expand_tiles(it, end, track_.tile_library(), std::back_inserter(tile_expansion_cache_));
      return tile_expansion_cache_;
    }


    void EditorScene::append_tile(resources::TrackLayer* layer, const resources::Tile& tile)
    {
      if (auto tiles = layer->tiles())
      {
        auto tile_index = static_cast<std::uint32_t>(tiles->size());
        tiles->push_back(tile);

        if (render_scene_)
        {
          const auto& tile_expansion = expand_tile(tile);
          render_scene_->add_tile(layer, tile_expansion.data(), tile_expansion.size());
        }
      }
    }

    void EditorScene::remove_tile(resources::TrackLayer* layer, std::uint32_t tile_index)
    {
      if (auto tiles = layer->tiles())
      {
        tiles->erase(tiles->begin() + tile_index);

        if (render_scene_)
        {
          auto& expansion = expand_tiles(tiles->begin(), tiles->end());
          render_scene_->rebuild_tile_layer_geometry(layer, expansion.data(), expansion.size());
        }
      }
    }

    void EditorScene::remove_last_tile(resources::TrackLayer* layer)
    {
      if (auto tiles = layer->tiles())
      {
        if (!tiles->empty())
        {
          remove_tile(layer, static_cast<std::uint32_t>(tiles->size()) - 1);
        }
      }
    }

    void EditorScene::rebuild_path_geometry(resources::TrackPath* path)
    {
      if (!render_scene_) return;

      for (auto& layer : track_.layers())
      {
        auto style = layer.path_style();
        if (style && style->path == path)
        {
          render_scene_->rebuild_path_layer_geometry(&layer);
        }
      }
    }

    void EditorScene::rebuild_path_layer(resources::TrackLayer* layer)
    {
      if (!render_scene_ || !layer->path_style()) return;

      render_scene_->rebuild_path_layer_geometry(layer);
    }
  }
}