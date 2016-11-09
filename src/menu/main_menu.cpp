/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "main_menu.hpp"
#include "update_input_state.hpp"

#include "user_interface/gui_layout.hpp"
#include "user_interface/gui_widgets.hpp"
#include "user_interface/gui_widget_events.hpp"
#include "user_interface/gui_style.hpp"
#include "user_interface/gui_background_style.hpp"
#include "user_interface/gui_text_style.hpp"
#include "user_interface/gui_geometry.hpp"

#include "resources/resource_store.hpp"

#include "fonts/font_library.hpp"
#include "fonts/builtin_fonts.hpp"

namespace ts
{
  namespace menu
  {
    struct MainMenu::Impl
    {
      Impl(resources::ResourceStore* resource_store)
        : resource_store(resource_store)
      {}

      resources::ResourceStore* resource_store;
      gui::Geometry geometry_cache_;
    };

    MainMenu::MainMenu(resources::ResourceStore* resource_store)
      : impl_(std::make_unique<Impl>(resource_store))
    {
    }

    MainMenu::~MainMenu() = default;

    void MainMenu::update(const gui::InputState& input_state, std::uint32_t frame_duration)
    {
      auto& geometry = impl_->geometry_cache_;
      geometry.text.clear();
      geometry.geometry.clear();

      auto button_placement = gui::vertical_layout_generator({ 100.0f, 100.0f }, { 150.0f, 30.0f });

      using namespace gui;
      auto font = impl_->resource_store->font_library().get_font_by_name(fonts::default_18);

      auto text_style = styles::text_style(*font, Colorb(0, 190, 0, 255), 
                                           Vector2f(), styles::center_text_horizontal);
      auto hover_text_style = text_style;
      hover_text_style.text_color = { 255, 255, 255, 255 };

      auto button_style = text_style + make_hover_style(hover_text_style +
                                                        styles::fill_area(Colorb(0, 100, 0, 255)));

      widgets::button("began", button_placement(), button_style, input_state, geometry,
                      events::on_click([]() {}));
    }

    void MainMenu::render(const gui::Renderer& renderer, const gui::RenderState& render_state) const
    {
      draw(renderer, impl_->geometry_cache_, render_state);
    }
  }
}