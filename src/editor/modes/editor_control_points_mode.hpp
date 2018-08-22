/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "editor/editor_mode.hpp"

#include "utility/vector2.hpp"

#include <boost/optional.hpp>

#include <cstdint>


namespace ts
{
  namespace resources
  {
    class Track;
  }

  namespace editor
  {
    class ControlPointsMode
      : public EditorMode
    {
    public:
      virtual const char* mode_name() const override;

      virtual void delete_last(const EditorContext& context) override;
      virtual void delete_selected(const EditorContext& context) override;

      virtual void update_tool_info(const EditorContext& context) override;
      virtual void update_canvas_interface(const EditorContext& context) override;

      virtual void activate(const EditorContext& context) override;

      virtual void next(const EditorContext& context) override;
      virtual void previous(const EditorContext& context) override;

    private:
      std::uint32_t selected_point_id_ = -1;
      boost::optional<Vector2i> added_point_position_;
    };
  }
}