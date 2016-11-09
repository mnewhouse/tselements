/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"

#include "track_editor_menu.hpp"
#include "track_editor_interface_state.hpp"

#include "user_interface/gui_layout.hpp"
#include "user_interface/gui_widgets.hpp"
#include "user_interface/gui_widget_events.hpp"
#include "user_interface/gui_background_style.hpp"
#include "user_interface/gui_style.hpp"

#include "fonts/font_library.hpp"
#include "fonts/builtin_fonts.hpp"

namespace ts
{
  namespace editor
  {
    namespace track
    {
      namespace detail
      {
        const Vector2f menu_item_size = { 150.0f, 20.0f };

        const auto menu_bar_gradient_colors = std::make_pair(Colorb(200, 200, 200, 255),
                                                             Colorb(150, 150, 150, 255));

        const auto menu_bar_hover_gradient_colors = std::make_pair(Colorb(255, 255, 220, 255),
                                                                   Colorb(255, 255, 150, 255));

        const auto menu_item_active_colors = std::make_pair(Colorb(200, 200, 200, 255),
                                                            Colorb(255, 255, 200, 255));

        auto menu_bar_item_style(const fonts::FontLibrary& font_library)
        {
          using namespace gui;

          auto font = font_library.get_font_by_name(fonts::default_18);

          auto text_style = styles::text_style(*font, Colorb(20, 20, 20, 255),
                                               Vector2f(0, 0), styles::center_text_horizontal);

          auto hover_text_style = text_style;
          hover_text_style.text_color = { 0, 0, 0, 255 };

          auto area_colors = menu_bar_gradient_colors;
          auto area_hover_colors = menu_bar_hover_gradient_colors;

          return text_style + styles::vertical_gradient(area_colors.first, area_colors.second) +
            make_hover_style(hover_text_style +
                             styles::vertical_gradient(area_hover_colors.first,
                                                       area_hover_colors.second));
        }

        auto menu_item_style(const fonts::FontLibrary& font_library)
        {
          using namespace gui;

          auto font = font_library.get_font_by_name(fonts::default_18);
          auto text_style = styles::text_style(*font, Colorb(0, 0, 0, 255), Vector2f(8.0f, 0.0f));

          auto hover_text_style = text_style;
          hover_text_style.text_color = { 0, 0, 0, 255 };

          return text_style + styles::fill_area(menu_bar_gradient_colors.first) +
            make_hover_style(hover_text_style +
                             styles::vertical_gradient(menu_bar_hover_gradient_colors.first,
                                                       menu_bar_hover_gradient_colors.second));
        }

        auto menu_active_item_style(const fonts::FontLibrary& font_library)
        {
          using namespace gui;

          auto font = font_library.get_font_by_name(fonts::default_18);
          auto text_style = styles::text_style(*font, Colorb(20, 20, 20, 255), Vector2f(8.0f, 0.0f));

          auto hover_text_style = text_style;
          hover_text_style.text_color = { 0, 0, 0, 255 };

          return text_style + styles::fill_area(menu_item_active_colors.first) +
            make_hover_style(hover_text_style +
                             styles::fill_area(menu_item_active_colors.second));
        }
      }

      Menu::Menu(const fonts::FontLibrary* font_library)
        : font_library_(font_library)
      {
        menu_expansion_state_.reserve(8);
      }

      Menu::UpdateState
        Menu::update(InterfaceState& interface_state, Vector2u screen_size,
                     const gui::InputState& input_state, gui::Geometry& geometry)
      {
        using namespace gui;

        interface_state_ = &interface_state;
        screen_size_ = screen_size;
        geometry_ = &geometry;
        input_state_ = &input_state;

        // Update menu bar
        auto menu_placement = gui::grid_layout_generator({ 0.0f, 0.0f }, { 80.0f, 20.0f },
                                                         static_cast<float>(screen_size.x));

        auto menu_bar_item_style = detail::menu_bar_item_style(*font_library_);



        using self = Menu;

        struct MenuItem
        {
          using menu_function = void (self::*)(FloatRect area);

          const char* label;
          menu_function func;
          FloatRect area;
        };

        const MenuItem menu_items[] =
        {
          { "File", nullptr, menu_placement() },
          { "Edit", nullptr, menu_placement() },
          { "Tools", &self::show_tools_menu, menu_placement() },
          { "Modes", &self::show_modes_menu, menu_placement() },
          { "Test", &self::show_test_menu, menu_placement() }
        };

        auto menu_bar_area = [&]()
        {
          FloatRect area = { 0.0f, 0.0f, static_cast<float>(screen_size.x), 1.0f };
          for (const auto& item : menu_items)
          {
            area = combine(area, item.area);
          }

          return area;
        }();

        auto menu_area_colors = detail::menu_bar_gradient_colors;
        gui::add_vertical_gradient(menu_bar_area, menu_area_colors.first,
                                   menu_area_colors.second, geometry);

        bool expansion_state = !menu_expansion_state_.empty();

        auto item_style = detail::menu_bar_item_style(*font_library_);

        std::uint32_t item_id = 0;
        for (const auto& item : menu_items)
        {
          auto state = widgets::button(item.label, item.area, item_style, input_state, geometry);

          if (!expansion_state && was_clicked(state) ||
              expansion_state && hover_state(state))
          {
            // Expand the menu
            menu_expansion_state_ = { item_id };
          }

          ++item_id;
        }

        // If there was a mouse click with a menu expansion already active,
        // hide the expansion. Otherwise, just show the menu expansion.
        if (!menu_expansion_state_.empty())
        {
          const auto& item = menu_items[menu_expansion_state_.front()];

          if (item.func)
          {
            (this->*item.func)(item.area);
          }
        }

        IntRect screen_rect(0, 0, screen_size.x, screen_size.y);
        if (expansion_state && was_clicked(widget_state(rect_cast<float>(screen_rect), input_state)))
        {
          menu_expansion_state_.clear();
        }

        UpdateState result;
        result.bounding_box = rect_cast<std::int32_t>(menu_bar_area);
        result.focus_state = expansion_state || !menu_expansion_state_.empty();
        return result;
      }

      void Menu::show_tools_menu(FloatRect area)
      {
        using namespace gui;

        struct MenuItem
        {
          const char* label;
          Tool tool_identifier;
        };

        MenuItem menu_items[] =
        {
          { "Path Tool", Tool::Path },
          { "Terrain Tool", Tool::Terrain },
          { "Elevation Tool", Tool::Elevation }
        };

        auto item_size = detail::menu_item_size;
        auto top_left = Vector2f(area.left, area.bottom());

        auto placement_generator = vertical_layout_generator(top_left, item_size);

        auto item_style = detail::menu_item_style(*font_library_);
        auto active_item_style = detail::menu_active_item_style(*font_library_);

        for (auto item : menu_items)
        {
          auto do_button = [&](auto&& style)
          {
            auto state = widgets::button(item.label, placement_generator(), style,
                                         *input_state_, *geometry_);

            if (was_clicked(state))
            {
              interface_state_->set_active_tool(item.tool_identifier);
            }
          };

          if (item.tool_identifier == interface_state_->active_tool()) do_button(active_item_style);
          else do_button(item_style);
        }
      }

      void Menu::show_modes_menu(FloatRect area)
      {
        using namespace gui;

        auto mode_names = mode_names_by_tool(interface_state_->active_tool());       

        auto item_size = detail::menu_item_size;
        auto top_left = Vector2f(area.left, area.bottom());

        auto placement_generator = vertical_layout_generator(top_left, item_size);

        auto item_style = detail::menu_item_style(*font_library_);
        auto active_item_style = detail::menu_active_item_style(*font_library_);

        std::size_t mode_id = 0;
        for (auto mode_name : mode_names)
        {
          auto do_button = [&](auto&& style)
          {
            auto state = widgets::button(mode_name, placement_generator(), style,
                                         *input_state_, *geometry_);

            if (was_clicked(state))
            {
              interface_state_->set_active_mode(mode_id);
            }
          };

          if (mode_id == interface_state_->active_mode()) do_button(active_item_style);
          else do_button(item_style);

          ++mode_id;
        }
      }

      void Menu::show_test_menu(FloatRect area)
      {
        using namespace gui;

        struct MenuItem
        {
          using menu_function = void (Menu::*)();

          const char* label;
          menu_function func;
        };

        MenuItem menu_items[] =
        {
          { "Setup...", &Menu::show_test_setup_window },
          { "Test!", &Menu::launch_test }
        };        

        auto item_size = detail::menu_item_size;
        auto top_left = Vector2f(area.left, area.bottom());

        auto placement_generator = vertical_layout_generator(top_left, item_size);

        auto item_style = detail::menu_item_style(*font_library_);
        auto active_item_style = detail::menu_active_item_style(*font_library_);

        for (auto menu_item : menu_items)
        {
          auto state = widgets::button(menu_item.label, placement_generator(), item_style,
                                       *input_state_, *geometry_);

          if (was_clicked(state))
          {
            (this->*menu_item.func)();
          }
        }
      }

      void Menu::launch_test()
      {
        interface_state_->set_active_state(StateId::Test);
      }

      void Menu::show_test_setup_window()
      {

      }
    }
  }
}