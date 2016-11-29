/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"
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

    const std::vector<resources::PlacedTile>& EditorScene::expand_tile(const resources::Tile& tile) const
    {
      tile_expansion_cache_.clear();
      resources::expand_tiles(&tile, &tile + 1, track_.tile_library(), std::back_inserter(tile_expansion_cache_));
      return tile_expansion_cache_;
    }

    void EditorScene::append_tile(resources::TrackLayer* layer, const resources::Tile& tile)
    {
      auto tile_index = layer->tiles().size();
      layer->tiles().push_back(tile);

      if (render_scene_)
      {   
        const auto& tile_expansion = expand_tile(tile);

        render_scene_->add_tile(layer, tile_index, tile_expansion.data(), tile_expansion.size());
      }
    }

    void EditorScene::remove_tile(resources::TrackLayer* layer, std::uint32_t tile_index)
    {
      auto& tiles = layer->tiles();
      tiles.erase(tiles.begin() + tile_index);

      if (render_scene_)
      {
        render_scene_->remove_tile(layer, tile_index);
      }
    }
  }
}