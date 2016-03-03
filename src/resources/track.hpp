/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef TRACK_HPP_192389
#define TRACK_HPP_192389

#include "tile_library.hpp"
#include "terrain_library.hpp"
#include "texture_library.hpp"
#include "track_layer.hpp"
#include "start_point.hpp"
#include "control_point.hpp"

#include "utility/vector2.hpp"

#include <boost/range/iterator_range.hpp>
#include <boost/iterator/indirect_iterator.hpp>

#include <memory>
#include <string>
#include <cstdint>
#include <cstddef>
#include <map>

namespace ts
{
  namespace resources
  {
    // The track class is a complete representation of a track.
    // It encompasses tiles, terrains, textures, vertices, control points, start points,
    // and more.
    class Track
    {
    public:
      Track() = default;

      void set_path(const std::string& path);
      const std::string& path() const noexcept;

      void set_author(std::string author);
      const std::string& author() const noexcept;

      Vector2u size() const;
      void set_size(const Vector2u& size);

      void set_height_level_count(std::uint32_t height_levels) noexcept;
      std::uint32_t height_level_count() const noexcept;

      TileLibrary& tile_library() noexcept;
      const TileLibrary& tile_library() const noexcept;

      TextureLibrary& texture_library() noexcept;
      const TextureLibrary& texture_library() const noexcept;

      TerrainLibrary& terrain_library() noexcept;
      const TerrainLibrary& terrain_library() const noexcept;

      using LayerId = std::uint32_t;
      TrackLayer* get_layer_by_id(LayerId layer_id);
      const TrackLayer* get_layer_by_id(LayerId layer_id) const;

      TrackLayer* create_layer(std::string layer_name, std::uint32_t level);
      std::size_t layer_count() const noexcept;

      using LayerOrderInterface = boost::iterator_range<boost::indirect_iterator<TrackLayer* const*>>;
      using ImmutableLayerOrderInterface = boost::iterator_range<boost::indirect_iterator<const TrackLayer* const*>>;

      LayerOrderInterface layers();
      ImmutableLayerOrderInterface layers() const;

      void add_control_point(const ControlPoint& point);
      const std::vector<ControlPoint>& control_points() const;

      void add_start_point(const StartPoint& point);
      const std::vector<StartPoint>& custom_start_points() const;

      const std::vector<StartPoint>& start_points() const;

    private:
      std::string path_;
      std::string author_;

      Vector2u size_;
      std::uint32_t height_level_count_;

      TileLibrary tile_library_;
      TerrainLibrary terrain_library_;
      TextureLibrary texture_library_;

      std::map<TrackLayer::Id, TrackLayer> layers_;
      std::vector<TrackLayer*> layer_order_;

      std::vector<ControlPoint> control_points_;
      std::vector<StartPoint> custom_start_points_;
      mutable std::vector<StartPoint> start_points_;
    };

    template<typename LayerType>
    struct LayerOrderInterface
    {
    private:
      friend Track;
      explicit LayerOrderInterface(LayerType** begin, LayerType** end);

      LayerType** begin_;
      LayerType** end_;
    };
  }
}

#endif