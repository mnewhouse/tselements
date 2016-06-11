/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef TRACK_3D_HPP_8192358125
#define TRACK_3D_HPP_8192358125

#include "height_map.hpp"
#include "texture_library_3d.hpp"
#include "track_path.hpp"

#include <utility/vector3.hpp>

#include <boost/range/iterator_range.hpp>
#include <boost/iterator/indirect_iterator.hpp>

#include <cstdint>
#include <map>

namespace ts
{
  namespace resources_3d
  {
    class Track
    {
    public:
      Vector3u size() const;
      Vector2u size_2d() const;
      void resize(Vector3u new_size);

      void update_height_map(HeightMap height_map);
      const HeightMap& height_map() const;

      const TextureLibrary& texture_library() const;

      void define_texture(std::uint16_t texture_id, std::string image_file,
                          IntRect image_rect);

      void set_base_texture(std::uint16_t texture_id);
      std::uint16_t base_texture() const;

      TrackPath* create_path();

      using path_range = boost::iterator_range<TrackPath* const*>;
      using const_path_range = boost::iterator_range<const TrackPath* const*>;

      path_range paths();
      const_path_range paths() const;

    private:
      Vector3u size_ = {};
      HeightMap height_map_;
      TextureLibrary texture_library_;
      std::uint16_t base_texture_ = 0;

      std::map<std::size_t, TrackPath> track_paths_;
      std::vector<TrackPath*> active_paths_;
    };
  }
}

#endif