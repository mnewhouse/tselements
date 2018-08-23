/**
** Top down racing game, codenamed TS3D
** Copyright © 2017 Martijn Nijhuis (mniehoes@gmail.com)
**
** This is proprietary software: modifying or distributing this source file
** in any shape or form is not allowed without explicit permission from the author.
**/

#pragma once

#include "resources/track_path.hpp"
#include "resources/geometry.hpp"

#include "utility/vector2.hpp"

#include <SFML/Graphics/Image.hpp>

#include <vector>
#include <cstdint>

namespace ts
{
  namespace scene
  {
    struct OutlinePoint
    {
      float time_point;
      Vector2f point;
      Vector2f normal;
      float width;
    };

    struct OutlineProperties
    {
      bool invert_normal = false;
      bool center_line = false;
      float width = 1.0f;
      float tolerance = 1.0f;
    };

    inline auto invert(OutlineProperties p)
    {
      p.invert_normal = !p.invert_normal;
      return p;
    }


    using PathVertex = resources::Vertex;
    using PathFace = resources::Face;

    void generate_path_segment_outline(const resources::SubPath& path, const resources::StrokeSegment& segment,
                                       const OutlineProperties& properties,
                                       std::vector<OutlinePoint>& outline_points);

    void generate_path_segment_outline(const resources::SubPath& path, const OutlineProperties& properties,
                                       std::vector<OutlinePoint>& outline_points);

    void create_path_geometry(const resources::TrackPath& path, const resources::PathStyle& path_style,
                              float tolerance, sf::Image& path_texture,
                              std::vector<PathVertex>& vertices, std::vector<PathFace>& faces);
  }
}