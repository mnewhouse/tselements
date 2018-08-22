/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "editor_context.hpp"

#include "game/game_events.hpp"

#include "utility/vector2.hpp"
#include "utility/rect.hpp"

#include <boost/range/iterator_range.hpp>

#include <SFML/Graphics/Transform.hpp>

namespace ts
{
  namespace scene
  {
    class Viewport;
  }

  namespace editor
  {
    class EditorMode
    {
    public:
      virtual ~EditorMode() = 0;

      void set_active_tool(std::uint32_t tool_id) { active_tool_ = tool_id; }
      std::uint32_t active_tool() const { return active_tool_; }
      
      virtual void update_tool_info(const EditorContext& context) {}
      virtual void update_canvas_interface(const EditorContext& context) {}

      virtual void delete_last(const EditorContext& context) {}
      virtual void delete_selected(const EditorContext& context) {}

      virtual void activate(const EditorContext& context) {}
      virtual void deactivate(const EditorContext& context) {}

      virtual void next(const EditorContext& context) {}
      virtual void previous(const EditorContext& context) {}

      virtual void on_canvas_render(const ImmutableEditorContext& context, const sf::Transform& matrix) const {}

      virtual const char* mode_name() const { return ""; }

      using tool_name_range = boost::iterator_range<const char* const*>;
      virtual tool_name_range tool_names() const { return tool_name_range(nullptr, nullptr); };

      

    private:
      std::uint32_t active_tool_ = 0;
    };

    inline EditorMode::~EditorMode() {}
  }
}