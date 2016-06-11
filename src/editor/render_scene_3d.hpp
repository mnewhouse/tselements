/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef RENDER_SCENE_HPP_5283598
#define RENDER_SCENE_HPP_5283598

#include "terrain_scene_3d.hpp"

#include "graphics/texture.hpp"
#include "graphics/shader.hpp"
#include "graphics/sampler.hpp"
#include "graphics/vertex_buffer.hpp"

#include "utility/vector3.hpp"

#include <glm/mat4x4.hpp>

#include <vector>

namespace ts
{
  namespace resources_3d
  {
    class Track;
    class HeightMap;

    struct TrackPath;
  }

  namespace scene_3d
  {
    class RenderScene
    {
    public:
      RenderScene();

      void render() const;

      void load_track_visuals(const resources_3d::Track& track);
      
      void set_camera_position(Vector2f position, float height_above_ground,
                               const resources_3d::HeightMap& height_map);

      void move_camera(Vector3f offset);
      void move_camera_2d(Vector2f offset, const resources_3d::HeightMap& height_map);
      Vector3f camera_position() const;

      static float max_drawing_distance();
      static float min_drawing_distance();

      glm::mat4 projection_matrix() const;
      glm::mat4 view_matrix() const;

      const TerrainScene& terrain_scene() const;
      void register_track_path(const resources_3d::TrackPath* path_path);

      void update(const resources_3d::TrackPath* track_path);
      void update(const resources_3d::TrackPath* track_path,
                  std::size_t node_index, std::size_t node_count);

    private:
      TerrainScene terrain_scene_;

      Vector3f camera_position_;
    };
  }
}

#endif