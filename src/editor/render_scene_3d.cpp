/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "render_scene_3d.hpp"
#include "scene_3d_shaders.hpp"
#include "track_3d.hpp"
#include "texture_library_3d.hpp"

#include "graphics/image_loader.hpp"

#include "utility/color.hpp"

#include <glm/gtx/transform.hpp>

#include <algorithm>

namespace ts
{
  namespace scene_3d
  {
    RenderScene::RenderScene()
      : terrain_scene_(),
        camera_position_({ 640.0f, 400.0f, 80.0f })
    {
    }

    void RenderScene::load_track_visuals(const resources_3d::Track& track)
    {
      terrain_scene_.load_track_terrains(track);

      set_camera_position(make_vector2(640.0f, 400.0f), 80.0f, track.height_map());
    }

    void RenderScene::render() const
    {
      auto view_mat = view_matrix();
      auto projection_mat = projection_matrix();

      terrain_scene_.render(view_mat, projection_mat);
    }

    glm::mat4x4 RenderScene::view_matrix() const
    {
      return glm::lookAt(glm::vec3(camera_position_.x, camera_position_.y, camera_position_.z),
                         glm::vec3(camera_position_.x, camera_position_.y, 0.0f),
                         glm::vec3(0.0f, 1.0f, 0.0f));
    }

    glm::mat4x4 RenderScene::projection_matrix() const
    {
      return glm::perspective(2.5f, 640.0f / 400.0f, min_drawing_distance(), max_drawing_distance());
    }

    void RenderScene::move_camera(Vector3f offset)
    {
      camera_position_ += offset;
    }

    float RenderScene::max_drawing_distance()
    {
      return 500.0f;
    }

    float RenderScene::min_drawing_distance()
    {
      return max_drawing_distance() / 250.0f;
    }

    Vector3f RenderScene::camera_position() const
    {
      return camera_position_;
    }

    const TerrainScene& RenderScene::terrain_scene() const
    {
      return terrain_scene_;
    }

    void RenderScene::register_track_path(const resources_3d::TrackPath* track_path)
    {
      terrain_scene_.register_track_path(track_path);
    }
    
    void RenderScene::update(const resources_3d::TrackPath* track_path)
    {
      terrain_scene_.update(track_path);
    }

    void RenderScene::update(const resources_3d::TrackPath* track_path,
                             std::size_t node_index, std::size_t node_count)
    {
      terrain_scene_.update(track_path, node_index, node_count);
    }

    void RenderScene::set_camera_position(Vector2f position, float height_above_ground,
                                          const resources_3d::HeightMap& height_map)
    {
      camera_position_.x = position.x;
      camera_position_.y = position.y;
      camera_position_.z = interpolate_height_at(height_map, position) + height_above_ground;
    }

    void RenderScene::move_camera_2d(Vector2f offset, const resources_3d::HeightMap& height_map)
    {
      // Move the camera according to a two-dimensional offset, maintaining the same height above
      // the terrain as before.

      auto old_height = interpolate_height_at(height_map, make_vector2(camera_position_.x,
                                                                       camera_position_.y));

      camera_position_.x += offset.x;
      camera_position_.y += offset.y;
      auto new_height = interpolate_height_at(height_map, make_vector2(camera_position_.x,
                                                                       camera_position_.y));

      camera_position_.z += new_height - old_height;
    }
  }
}