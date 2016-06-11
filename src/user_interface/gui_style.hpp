/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef GUI_STYLE_HPP_54781741124
#define GUI_STYLE_HPP_54781741124

#include <type_traits>

namespace ts
{
  namespace gui
  {
    namespace styles
    {
      template <typename Style>
      struct HoverStyle
      {
        Style hover_style;
      };

      template <typename Style>
      struct ClickStyle
      {
        Style click_style;
      };

      template <typename... Types>
      struct CompoundStyle
        : Types...
      {
        struct init_tag {};

        template <typename... Args>
        explicit CompoundStyle(init_tag, Args&&... args)
          : Types(std::forward<Args>(args))...
        {}
      };

      template <typename FirstStyle, typename SecondStyle>
      auto operator+(FirstStyle&& first, SecondStyle&& second)
      {
        using result_type = CompoundStyle<std::remove_reference_t<FirstStyle>,
          std::remove_reference_t<SecondStyle>>;

        return result_type(typename result_type::init_tag{},
                           std::forward<FirstStyle>(first), std::forward<SecondStyle>(second));
      }

      template <typename... First, typename Second>
      auto operator+(CompoundStyle<First...>&& first, Second&& second)
      {
        using result_type = CompoundStyle<First..., std::remove_reference_t<Second>>;
        return result_type(static_cast<First&&>(first)..., std::forward<Second>(second));
      }

      template <typename First, typename... Second>
      auto operator+(First&& first, CompoundStyle<Second...>&& second)
      {
        using result_type = CompoundStyle<std::remove_reference_t<First>, Second...>;
        return result_type(std::forward<First>(first), static_cast<Second&&>(second)...);
      }

      template <typename... First, typename Second>
      auto operator+(CompoundStyle<First...>&& first, CompoundStyle<Second...>&& second)
      {
        using result_type = CompoundStyle<First..., Second...>;
        return result_type(static_cast<First&&>(first)..., static_cast<Second&&>(second)...);
      }

      // Todo: const lvalue overloads if needed?
    }

    namespace detail
    {
      template <typename Style>
      auto hover_style(const Style& style, int) -> decltype(style.hover_style)
      {
        return style.hover_style;
      }

      template <typename Style>
      const auto& hover_style(const Style& style, ...)
      {
        return style;
      }

      template <typename Style>
      auto click_style(const Style& style, int) -> decltype(style.click_style)
      {
        return style.click_style;
      }

      template <typename Style>
      auto click_style(const Style& style, short) -> decltype(style.hover_style)
      {
        return style.hover_style;
      }

      template <typename Style>
      const auto& click_style(const Style& style, ...)
      {
        return style;
      }
    }

    template <typename Style>
    decltype(auto) hover_style(const Style& style)
    {
      return detail::hover_style(style, 0);
    }

    template <typename Style>
    decltype(auto) click_style(const Style& style)
    {
      return detail::click_style(style, 0);
    }

    template <typename Style>
    auto make_hover_style(Style&& style)
    {
      return styles::HoverStyle<std::remove_reference_t<Style>>{ std::forward<Style>(style) };
    }

    template <typename Style>
    auto make_click_style(Style&& style)
    {
      return styles::ClickStyle<std::remove_reference_t<Style>>{ std::forward<Style>(style) };
    }
  }
}

#endif