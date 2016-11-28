/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "track_scene.hpp"
#include "viewport.hpp"
#include "drawable_entity.hpp"

#include "graphics/shader.hpp"
#include "graphics/buffer.hpp"

#include "utility/color.hpp"
#include "utility/vector2.hpp"

#include <glm/mat2x2.hpp>
#include <glm/mat4x4.hpp>

#include <cstdint>
#include <functional>

namespace ts
{
  namespace scene
  {
    namespace render_scene
    {
      struct TrackLayerData
      {
        graphics::Buffer vertex_buffer;
        graphics::Buffer index_buffer;

        std::uint32_t vertex_buffer_size;
        std::uint32_t index_buffer_size;
      };

      struct TrackComponent
      {
        const TrackLayerData* layer_data;
        const graphics::Texture* texture;        

        std::uint32_t level;
        std::uint32_t element_buffer_offset;
        std::uint32_t element_count;
      };

      struct TrackComponentUniformLocations
      {
        std::uint32_t view_matrix;
        std::uint32_t texture_sampler;
      };

      struct CarUniformLocations
      {
        std::uint32_t frame_progress;
        std::uint32_t view_matrix;
        std::uint32_t model_matrix;
        std::uint32_t new_model_matrix;
        std::uint32_t colorizer_matrix;
        std::uint32_t car_colors;
        std::uint32_t texture_sampler;
        std::uint32_t colorizer_sampler;
      };

      struct BoundaryUniformLocations
      {
        std::uint32_t view_matrix;
        std::uint32_t world_size;
      };

      struct EntityInfo
      {
        const graphics::Texture* texture;
        glm::mat4 model_matrix;
        glm::mat4 new_model_matrix;
        glm::mat4 colorizer_matrix;
        std::array<float, 12> colors;
      };
    }

    class TrackScene;
    class DynamicScene;
    class ParticleGenerator;

    struct PostRenderCallback
    {
      virtual void operator()(const glm::mat4& view_matrix, IntRect screen_rect) const {}
    };

    class RenderScene
    {
    public:
      explicit RenderScene(TrackScene track_scene);

      using render_callback = std::function<void(const glm::mat4& view_matrix)>;
      // Render a viewport with an optional callback. The callback can be used to draw more stuff.
      // The scissor region, viewport and stencil buffer are retained when the callback is invoked, 
      // but no guarantees are made about anything else.
      void render(const Viewport& viewport, Vector2i screen_size, double frame_progress,
                  const render_callback& = nullptr) const;

      void clear_dynamic_state();
      void update_entities(const DynamicScene& dynamic_scene, std::uint32_t frame_duration);
      void update_particles(const ParticleGenerator& particle_generator, std::uint32_t frame_duration);
      void set_background_color(Colorf bg_color);

      const TrackScene& track_scene() const;
      TrackScene& track_scene();

    private:
      void load_shader_programs();
      void setup_entity_buffers();
      void load_track_components(const TrackScene& track_scene);

      TrackScene track_scene_;

      graphics::ShaderProgram track_shader_program_;
      graphics::ShaderProgram car_shader_program_;
      graphics::ShaderProgram boundary_shader_program_;

      graphics::Buffer boundary_index_buffer_;
      graphics::Buffer boundary_vertex_buffer_;      
      
      graphics::Buffer car_vertex_buffer_;
      graphics::Buffer car_index_buffer_;
      
      render_scene::TrackComponentUniformLocations track_component_uniform_locations_;
      render_scene::CarUniformLocations car_uniform_locations_;
      render_scene::BoundaryUniformLocations boundary_uniform_locations_;

      std::unordered_map<const resources::TrackLayer*, render_scene::TrackLayerData> layers_;
      std::vector<render_scene::TrackComponent> track_components_;
      std::vector<render_scene::EntityInfo> drawable_entities_;
      std::vector<DrawableEntity::Vertex> car_vertex_buffer_cache_;

      Colorf background_color_ = Colorf(0.f, 0.f, 0.f, 1.0f);
    };
  }
}
