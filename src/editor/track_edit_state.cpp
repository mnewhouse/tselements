/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "track_edit_state.hpp"

#include "graphics/render_window.hpp"

#include "user_interface/gui_context.hpp"
#include "user_interface/gui_layout.hpp"
#include "user_interface/gui_widgets.hpp"
#include "user_interface/gui_background_style.hpp"
#include "user_interface/gui_style.hpp"

#include "menu/update_input_state.hpp"

#include "resources/resource_store.hpp"

#include "fonts/font_library.hpp"
#include "fonts/builtin_fonts.hpp"

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
        auto height_map = resources_3d::generate_height_map(128, 16, 0.0f, 0.0f);

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

        // path->strokes.push_back(tarmac_edge);
        path->strokes.push_back(track_edge);
        path->strokes.push_back(tarmac);

        return track;        
      }
    }

    TrackEditState::TrackEditState(const game_context& ctx)
      : GameState(ctx),
        editor_scene_(detail::load_track()),
        path_tool_(&editor_scene_),
        active_tool_(&path_tool_),
        input_state_()
    {
      editor_scene_.load_scene();
    }

    void TrackEditState::render(const render_context& ctx) const
    {
      editor_scene_.render();

      if (active_tool_) active_tool_->render();

      auto game_context = context();
      const auto& gui_renderer = game_context.gui_context->renderer();

      gui::RenderState render_state;
      render_state.screen_size = game_context.render_window->size();
      render_state.translation = { 0.0f, 0.0f };

      gui::draw(gui_renderer, gui_geometry_, render_state);
    }

    void TrackEditState::update(const update_context& ctx)
    {
      // Update GUI
      clear(gui_geometry_);

      auto screen_size = context().render_window->size();
      editor_scene_.set_view_port(screen_size, IntRect(100, 100, screen_size.x - 100, screen_size.y - 100));

      using namespace gui;
      auto resource_store = context().resource_store;
      auto font = resource_store->font_library().get_font_by_name(fonts::default_18);
      
      // Update menu bar
      auto menu_placement = gui::vertical_layout_generator({ 0.0f, 0.0f }, { 150.0f, 20.0f });

      auto menu_text_style = styles::text_style(*font, Colorb(20, 20, 20, 255),
                                                styles::center_text_horizontal);

      auto menu_hover_text_style = menu_text_style;
      menu_hover_text_style.text_color = { 0, 0, 0, 255 };

      auto button_style = menu_text_style + styles::fill_area(Colorb(150, 150, 150, 255)) +
        make_hover_style(menu_hover_text_style + styles::fill_area(Colorb(255, 255, 150, 255)));

      widgets::button("Menu", menu_placement(), button_style, input_state_,
                      gui_geometry_);
    }

    void TrackEditState::process_event(const event_type& event)
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

      if (active_tool_) active_tool_->process_event(event);

      menu::update_input_state(input_state_, event);
    }
  }
}