/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "tile_library.hpp"
#include "terrain_library.hpp"
#include "texture_library.hpp"
#include "path_library.hpp"
#include "track_layer.hpp"
#include "start_point.hpp"
#include "control_point.hpp"
#include "track_path.hpp"

#include "utility/vector2.hpp"

#include <boost/range/iterator_range.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/iterator/indirect_iterator.hpp>

#include <vector>
#include <list>
#include <memory>
#include <string>
#include <cstdint>
#include <cstddef>
#include <map>

namespace ts
{
  namespace resources
  {
    namespace detail
    {
      struct ConstPathRangeTransform
      {
        const resources::TrackPath* operator()(const std::unique_ptr<TrackPath>& p) const
        {
          return p.get();
        }
      };

      struct PathRangeTransform
      {
        resources::TrackPath* operator()(const std::unique_ptr<TrackPath>& p) const
        {
          return p.get();
        }
      };      
    }

    // The track class is a complete representation of a track.
    // It encompasses tiles, terrains, textures, vertices, control points, start points,
    // and more.
    class Track
    {
    public:
      Track() = default;

      Track(const Track&) = default;
      Track& operator=(const Track&) = default;

      Track(Track&&) = default;
      Track& operator=(Track&&) = default;

      void set_path(const std::string& path);
      const std::string& path() const noexcept;

      void set_author(std::string author);
      const std::string& author() const noexcept;

      Vector2i size() const;
      void set_size(const Vector2i& size);

      void set_height_level_count(std::int32_t height_levels) noexcept;
      std::int32_t height_level_count() const noexcept;

      TileLibrary& tile_library() noexcept;
      const TileLibrary& tile_library() const noexcept;

      TextureLibrary& texture_library() noexcept;
      const TextureLibrary& texture_library() const noexcept;

      TerrainLibrary& terrain_library() noexcept;
      const TerrainLibrary& terrain_library() const noexcept;

      PathLibrary& path_library() noexcept;
      const PathLibrary& path_library() const noexcept;

      using LayerId = std::uint32_t;
      TrackLayer* create_layer(TrackLayerType type, std::string layer_name, std::uint32_t level);

      void deactivate_layer(TrackLayer* layer);
      void activate_layer(TrackLayer* layer);        

      std::size_t layer_count() const noexcept;

      void set_layer_level(TrackLayer* layer, std::uint32_t level);
      std::int32_t shift_towards_front(const TrackLayer* layer, std::int32_t amount = 1);
      std::int32_t shift_towards_back(const TrackLayer* layer, std::int32_t amount = 1);

      using LayerOrderInterface = boost::iterator_range<boost::indirect_iterator<TrackLayer* const*>>;
      using ConstLayerOrderInterface = boost::iterator_range<boost::indirect_iterator<const TrackLayer* const*>>;

      LayerOrderInterface layers();
      ConstLayerOrderInterface layers() const;      

      void add_control_point(const ControlPoint& point);
      void add_control_point(const ControlPoint& point, std::size_t idx);
      const std::vector<ControlPoint>& control_points() const;
      void remove_control_point(std::size_t idx);
      void update_control_point(std::size_t idx, const ControlPoint& cp);

      void add_start_point(const StartPoint& point);
      const std::vector<StartPoint>& custom_start_points() const;

      const std::vector<StartPoint>& start_points() const;

      void add_asset(const std::string& path);
      const std::vector<std::string>& assets() const;

    private:
      void insert_layer(TrackLayer* layer);
      void update_z_index();

      std::string path_;
      std::string author_;

      Vector2i size_ = {};
      std::int32_t height_level_count_ = 1;

      TileLibrary tile_library_;
      TerrainLibrary terrain_library_;
      TextureLibrary texture_library_;
      PathLibrary path_library_;

      std::vector<std::string> assets_;

      std::map<LayerId, TrackLayer> layers_;
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
