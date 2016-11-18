/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "editor_context.hpp"

#include "game/game_events.hpp"

#include "utility/vector2.hpp"
#include "utility/rect.hpp"

#include <boost/range/iterator_range.hpp>

namespace ts
{
  namespace scene
  {
    class Viewport;
  }

  namespace editor
  {
    class EditorTool
    {
    public:
      virtual ~EditorTool() = 0;

      void set_active_mode(std::uint32_t mode_id) { active_mode_ = mode_id; }
      std::uint32_t active_mode() const { return active_mode_; }
      
      virtual void update_tool_info(const EditorContext& context) {}
      virtual void update_canvas_interface(const EditorContext& context) {}

      virtual void delete_last(const EditorContext& context) {}
      virtual void delete_selected(const EditorContext& context) {}

      virtual const char* tool_name() const { return "Tool"; }

      using mode_name_range = boost::iterator_range<const char* const*>;
      virtual mode_name_range mode_names() const { return mode_name_range(nullptr, nullptr); };

    private:
      std::uint32_t active_mode_ = 0;
    };

    inline EditorTool::~EditorTool() {}
  }
}