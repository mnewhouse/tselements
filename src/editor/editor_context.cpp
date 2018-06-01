/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "editor_context.hpp"

#include "scene/viewport.hpp"

namespace ts
{
  namespace editor
  {
    CoordTransform::CoordTransform(const ImmutableEditorContext& context)
      : screen_rect_(rect_cast<double>(context.canvas_viewport.screen_rect())),
        camera_center_(scene::compute_camera_center(context.canvas_viewport.camera(), context.world_size,
                                                    size(screen_rect_), 0)),
        zoom_(context.canvas_viewport.camera().zoom_level()),
        inverse_zoom_(1.0 / zoom_)
    {
    }

    Vector2d CoordTransform::world_position(Vector2d canvas_position) const
    {
      return (canvas_position - size(screen_rect_) * 0.5) * inverse_zoom_ + camera_center_;
    }

    Vector2d CoordTransform::viewport_position(Vector2d world_position) const
    {
      return (world_position - camera_center_) * zoom_ + size(screen_rect_) * 0.5;
    }

    Vector2d calculate_world_position(const ImmutableEditorContext& context, Vector2d viewport_position)
    {
      CoordTransform transform(context);
      return transform.world_position(viewport_position);
    }

    Vector2d calculate_viewport_position(const ImmutableEditorContext& context, Vector2d world_position)
    {
      CoordTransform transform(context);
      return transform.viewport_position(world_position);
    }
  }
}