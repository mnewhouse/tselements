/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef RENDER_SCENE_HPP_43289721
#define RENDER_SCENE_HPP_43289721

#include "track_scene.hpp"
#include "viewport.hpp"
#include "drawable_entity.hpp"

#include "graphics/shader.hpp"
#include "graphics/buffer.hpp"

#include "utility/color.hpp"

#include <glm/mat2x2.hpp>
#include <glm/mat4x4.hpp>

namespace ts
{
  namespace scene
  {
    class TrackScene;
    class DynamicScene;
    class ParticleGenerator;

    class RenderScene
    {
    public:
      explicit RenderScene(TrackScene track_scene);

      void render(const Viewport& viewport, Vector2u screen_size, double frame_progress) const;

      void register_scene_models(const DynamicScene& dynamic_scene);

      void update_entities(const DynamicScene& dynamic_scene, std::uint32_t frame_duration);
      void update_particles(const ParticleGenerator& particle_generator, std::uint32_t frame_duration);

    private:
      void load_shader_programs();
      void setup_entity_buffers();
      void load_track_components(const TrackScene& track_scene);

      TrackScene track_scene_;

      graphics::ShaderProgram track_shader_program_;
      graphics::ShaderProgram car_shader_program_;
      
      graphics::Buffer track_component_vertex_buffer_;
      graphics::Buffer track_component_element_buffer_;

      graphics::Buffer car_vertex_buffer_;
      graphics::Buffer car_index_buffer_;

      struct TrackComponent
      {
        const graphics::Texture* texture;

        std::uint32_t level;
        std::uint32_t element_buffer_offset;
        std::uint32_t element_count;
      };

      std::vector<TrackComponent> track_components_;
      
      struct TrackComponentUniformLocations
      {
        std::uint32_t view_matrix;
        std::uint32_t texture_sampler;
      } track_component_uniform_locations_;

      struct CarUniformLocations
      {
        std::uint32_t view_matrix;
        std::uint32_t model_matrix;
        std::uint32_t new_model_matrix;
        std::uint32_t colorizer_matrix;
        std::uint32_t frame_progress;
        std::uint32_t car_colors;
        std::uint32_t texture_sampler;
        std::uint32_t colorizer_sampler;
      } car_uniform_locations_;

      struct EntityInfo
      {
        const graphics::Texture* texture;
        glm::mat4 model_matrix;
        glm::mat4 new_model_matrix;
        glm::mat4 colorizer_matrix;
        std::array<float, 12> colors;
      };

      std::vector<EntityInfo> drawable_entities_;
      std::vector<DrawableEntity::Vertex> car_vertex_buffer_cache_;
    };
  }
}

#endif