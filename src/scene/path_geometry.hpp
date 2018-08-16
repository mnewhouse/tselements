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
    using PathVertex = resources::Vertex;
    using PathFace = resources::Face;

    void create_path_geometry(const resources::TrackPath& path, const resources::PathStyle& path_style,
                              float tolerance, sf::Image& path_texture,
                              std::vector<PathVertex>& vertices, std::vector<PathFace>& faces);

    /*
    void create_border_geometry(const std::vector<OutlinePoint>& outline, OutlineIndices outline_indices,
                                const resources::BorderStyle& border_style, Vector2f texture_size,
                                std::vector<PathVertex>& vertices, std::vector<PathFace>& faces);

    RenderedPath render_path(const std::vector<OutlinePoint>& path_outline,
                             Vector2i track_size, int cell_bits);
                             */
  }
}