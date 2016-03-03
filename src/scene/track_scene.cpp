/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "track_scene.hpp"

namespace ts
{
  namespace scene
  {
    TrackScene::TrackScene(Vector2u track_size)
      : track_size_(track_size)
    {
    }

    Vector2u TrackScene::track_size() const
    {
      return track_size_;
    }

    const TrackScene::texture_type* TrackScene::register_texture(std::unique_ptr<texture_type> texture)
    {
      textures_.push_back(std::move(texture));
      return textures_.back().get();
    }

    const TrackScene::texture_type* TrackScene::texture(std::size_t index) const
    {
      return textures_[index].get();
    }

    std::size_t TrackScene::texture_count() const
    {
      return textures_.size();
    }
  }
}