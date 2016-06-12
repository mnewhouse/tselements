/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef GUI_EVENT_HANDLERS_HPP_48129834
#define GUI_EVENT_HANDLERS_HPP_48129834

#include "gui_input_state.hpp"

#include "utility/rect.hpp"

#include <type_traits>

namespace ts
{
  namespace gui
  {
    template <typename Func>
    struct EventHandler
    {
    public:
      explicit EventHandler(Func func)
        : func_(std::move(func))
      {}
      
      template <typename... Args>
      auto operator()(Args&&... args) const
      {
        return func_(std::forward<Args>(args)...);
      }

    private:
      Func func_;
    };

    namespace events
    {
#define GUI_DEFINE_EVENT(class_name, helper_func_name) template <typename Func> \
      struct class_name : EventHandler<Func> \
       { \
         using EventHandler::EventHandler; \
       }; \
       template <typename Func> \
       auto helper_func_name(Func&& func) \
       { \
         return class_name<std::remove_reference_t<Func>>(std::forward<Func>(func)); \
       }

      GUI_DEFINE_EVENT(OnClick, on_click)

#undef GUI_DEFINE_EVENT

      template <typename WidgetState, typename Func>
      void process_event(const WidgetState& widget_state, const events::OnClick<Func>& event)
      {
        if (was_clicked(widget_state))
        {
          event();
        }
      }
    }
  }
}

#endif