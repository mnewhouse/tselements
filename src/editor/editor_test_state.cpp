/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "editor_test_state.hpp"

#include "graphics/render_window.hpp"

#include <iostream>

namespace ts
{
  namespace editor
  {
    namespace detail
    {
      auto load_track()
      {
        resources_3d::Track track;
        auto height_map = resources_3d::generate_height_map(128, 16, 64.0f, 0.6f);

        track.update_height_map(std::move(height_map));
        track.resize({ 1280, 800, 128 });

        track.define_texture(0, "editor/grass.png", IntRect(0, 0, 512, 512));
        track.define_texture(1, "editor/tarmac.png", IntRect(0, 0, 512, 512));
        track.define_texture(2, "editor/track_edge.png", IntRect(0, 0, 512, 512));
        track.set_base_texture(0);

        auto path = track.create_path();
        resources_3d::TrackPathStroke tarmac;
        tarmac.texture_id = 1;       

        resources_3d::TrackPathStroke track_edge;
        track_edge.type = resources_3d::TrackPathStroke::Border;
        track_edge.use_relative_size = false;
        track_edge.width = 2.5f;
        track_edge.offset = -2.0f;
        track_edge.texture_id = 2;

        resources_3d::TrackPathStroke tarmac_edge;
        tarmac_edge.type = resources_3d::TrackPathStroke::Border;
        tarmac_edge.use_relative_size = false;
        tarmac_edge.width = 1.5f;
        tarmac_edge.offset = -3.0f;
        tarmac_edge.texture_id = 1;
        tarmac_edge.color = { 150, 150, 150, 255 };

        path->strokes.push_back(tarmac_edge);
        path->strokes.push_back(track_edge);
        path->strokes.push_back(tarmac);

        return track;        
      }
    }

    TestState::TestState(const game_context& ctx)
      : GameState(ctx),
        editor_scene_(detail::load_track()),
        path_tool_(&editor_scene_),
        active_tool_(&path_tool_)
    {
      editor_scene_.load_scene();
    }

    void TestState::render(const render_context&) const
    {
      editor_scene_.render();

      path_tool_.render();
    }

    void TestState::process_event(const event_type& event)
    {
      if (event.type == sf::Event::KeyPressed)
      {
        switch (event.key.code)
        {
        case sf::Keyboard::Add:
          editor_scene_.move_camera({ 0.0f, 0.0f, -2.0f });
          break;

        case sf::Keyboard::Subtract:
          editor_scene_.move_camera({ 0.0f, 0.0f, 2.0f });
          break;

        case sf::Keyboard::Left:
          editor_scene_.move_camera_2d({ -5.0f, 0.0f });
          break;

        case sf::Keyboard::Right:
          editor_scene_.move_camera_2d({ 5.0f, 0.0f });
          break;

        case sf::Keyboard::Up:
          editor_scene_.move_camera_2d({ 0.0f, 5.0f });
          break;

        case sf::Keyboard::Down:
          editor_scene_.move_camera_2d({ 0.0f, -5.0f });
          break;
        }
      }

      else if (event.type == sf::Event::MouseMoved)
      {
      }

      if (active_tool_) active_tool_->process_event(event);
    }
  }
}