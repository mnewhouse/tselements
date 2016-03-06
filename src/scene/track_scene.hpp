/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef TRACK_SCENE_HPP_88859312
#define TRACK_SCENE_HPP_88859312

#include "graphics/texture.hpp"

#include "utility/vertex.hpp"
#include "utility/vertex_interface.hpp"

#include "resources/geometry.hpp"

#include <boost/iterator/transform_iterator.hpp>

#include <vector>
#include <memory>

namespace ts
{
  namespace scene
  {
    // TrackScene represents the drawable portion of the track.
    // It keeps the textures that are to be used, and a list of vertices
    // in the order that they must be drawn.
    class TrackScene
      : public utility::VertexInterface<const graphics::Texture*, resources::Vertex>
    {
    public:
      explicit TrackScene(Vector2u track_size);

      using texture_type = graphics::Texture;
      const texture_type* register_texture(std::unique_ptr<texture_type> texture);

      const texture_type* texture(std::size_t index) const;
      std::size_t texture_count() const;

      Vector2u track_size() const;

    private:
      std::vector<std::unique_ptr<texture_type>> textures_;
      Vector2u track_size_;
    };
  }
}

#endif