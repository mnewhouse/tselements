/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
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
#include "utility/rect.hpp"

#include <SFML/Graphics/Transform.hpp>

#include <cstdint>
#include <functional>
#include <array>

namespace ts
{
  namespace resources
  {
    struct TrackPath;
    class TrackLayer;
  }

  namespace scene
  {
    struct ParticleVertex
    {
      Vector2f position;
      Vector2f texture_coords;
      Colorb color;
    };

    namespace render_scene
    {
      struct TrackLayerData
      {
        TrackLayerData();

        graphics::Buffer vertex_buffer;
        graphics::Buffer index_buffer;
        graphics::VertexArray vertex_array;

        std::size_t vertex_buffer_size = 0;
        std::size_t index_buffer_size = 0;

        const TrackSceneLayer* scene_layer = nullptr;
      };

      struct TrackComponent
      {
        enum Type
        {
          Default,
          Path
        };

        const TrackLayerData* layer_data;
        std::array<const graphics::Texture*, 3> textures;
        std::array<Vector2f, 3> texture_scales;

        Type type = Default;
        std::uint32_t level;
        std::uint32_t z_index;
        std::uint32_t element_buffer_offset;
        std::uint32_t element_count;

        FloatRect bounding_box;
      };



      struct TrackComponentLocations
      {
        std::uint32_t view_matrix;
        std::uint32_t texture_sampler;        
        std::uint32_t min_corner;
        std::uint32_t max_corner;
      };

      struct TrackPathComponentLocations
      {
        std::uint32_t view_matrix;
        std::uint32_t weight_sampler;
        std::uint32_t primary_sampler;
        std::uint32_t secondary_sampler;
        std::uint32_t primary_scale;
        std::uint32_t secondary_scale;
        std::uint32_t min_corner;
        std::uint32_t max_corner;
        std::uint32_t z_base;
        std::uint32_t z_scale;
      };

      struct BoundaryLocations
      {
        std::uint32_t view_matrix;
        std::uint32_t world_size;
      };      

      struct CarLocations
      {
        std::uint32_t frame_progress;
        std::uint32_t view_matrix;
        std::uint32_t model_matrix;
        std::uint32_t new_model_matrix;
        std::uint32_t texture_coords_offset;
        std::uint32_t texture_coords_scale;
        std::uint32_t colorizer_matrix;        
        std::uint32_t car_colors;
        std::uint32_t texture_sampler;
        std::uint32_t colorizer_sampler;
      };

      struct ParticleLocations
      {
        std::uint32_t view_matrix;
        std::uint32_t texture_sampler;
      };
    }

    class TrackScene;
    class DynamicScene;
    class ParticleGenerator;

    struct PostRenderCallback
    {
      virtual void operator()(const sf::Transform& view_matrix, IntRect screen_rect) const {}
    };

    class RenderScene
    {
    public:
      RenderScene() = default;
      explicit RenderScene(TrackScene track_scene);

      using render_callback = std::function<void(const sf::Transform& view_matrix)>;
      // Render a viewport with an optional callback. The callback can be used to draw more stuff.
      // The scissor region, viewport and stencil buffer are retained when the callback is invoked, 
      // but no guarantees are made about anything else.
      void render(const Viewport& viewport, Vector2i screen_size, double frame_progress,
                  const render_callback& = nullptr) const;

      void clear_dynamic_state();
      void update_entities(const DynamicScene& dynamic_scene);
      void update_particles(const ParticleGenerator& particle_generator);

      void set_background_color(Colorf bg_color);

      const TrackScene& track_scene() const;
      
      void add_tile(const resources::TrackLayer* layer,
                    const resources::PlacedTile* tile_expansion, std::size_t tile_count);

      void rebuild_path_layer_geometry(const resources::TrackLayer* path_layer);
      void rebuild_tile_layer_geometry(const resources::TrackLayer* tile_layer,
                                       const resources::PlacedTile* tile_expansion, std::size_t tile_count);

      void reorder_track_components();      

    private:
      void load_shader_programs();
      void setup_entity_buffers(); 
      void setup_vertex_arrays();
      void setup_particle_buffers(std::uint32_t num_levels, std::uint32_t max_particles);
      void update_track_vertex_arrays();
      void load_particle_texture();

      void update_layer_geometry(const resources::TrackLayer* layer);

      void reload_track_components();

      TrackScene track_scene_;

      graphics::ShaderProgram track_shader_program_;
      graphics::ShaderProgram track_path_shader_program_;
      graphics::ShaderProgram car_shader_program_;
      graphics::ShaderProgram boundary_shader_program_;
      graphics::ShaderProgram particle_shader_program_;
      
      graphics::Buffer car_vertex_buffer_;
      graphics::Buffer car_index_buffer_;
      graphics::VertexArray car_vertex_array_;

      graphics::Buffer boundary_vertex_buffer_;
      graphics::Buffer boundary_index_buffer_;
      graphics::VertexArray boundary_vertex_array_;

      graphics::Buffer particle_vertex_buffer_;
      graphics::Buffer particle_index_buffer_;
      graphics::VertexArray particle_vertex_array_;
      graphics::Texture particle_texture_;
      
      render_scene::TrackComponentLocations track_component_locations_;
      render_scene::TrackPathComponentLocations track_path_component_locations_;
      render_scene::CarLocations car_locations_;
      render_scene::BoundaryLocations boundary_locations_;
      render_scene::ParticleLocations particle_locations_;

      std::unordered_map<const TrackSceneLayer*, render_scene::TrackLayerData> layers_;
      std::vector<render_scene::TrackComponent> track_components_;
      std::vector<DrawableEntity> drawable_entities_;

      bool first_time_setup_ = true;
      bool update_track_vaos_ = false;

      float z_level_increment_ = 0.0f;
      float z_index_increment_ = 0.0f;

      struct ParticleLevelInfo
      {
        std::uint32_t start_index = 0;
        std::uint32_t count = 0;        
      };

      std::uint32_t max_particles_per_level_ = 0;
      std::vector<ParticleLevelInfo> particle_level_info_;
      std::vector<ParticleVertex> particle_vertex_cache_;

      Colorf background_color_ = Colorf(0.f, 0.f, 0.f, 1.0f);      
    };
  }
}
