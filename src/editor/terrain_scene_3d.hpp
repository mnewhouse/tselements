/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef TERRAIN_SCENE_3D_HPP_854918289
#define TERRAIN_SCENE_3D_HPP_854918289

#include "graphics/vertex_buffer.hpp"
#include "graphics/texture.hpp"
#include "graphics/sampler.hpp"
#include "graphics/shader.hpp"

#include "path_vertices.hpp"

#include "utility/color.hpp"

#include <glm/mat4x4.hpp>

#include <cstdint>
#include <vector>
#include <cstddef>
#include <unordered_map>

namespace ts
{
  namespace resources_3d
  {
    class Track;
    class TextureLibrary;

    struct TrackPath;
    struct TrackPathNode;
  }

  namespace scene_3d
  {
    struct TerrainVertex
    {
      Vector2f position;
      Vector3f tex_coords;
      Colorb color;
    };

    class TerrainScene
    {
    public:
      TerrainScene();

      void load_track_terrains(const resources_3d::Track& track);

      void render(const glm::mat4x4& view_mat, const glm::mat4x4& projection_mat) const;

      const graphics::Texture& height_map_texture() const;
      float height_map_max_z() const;
      Vector2f height_map_cell_size() const;

      void register_track_path(const resources_3d::TrackPath* track_path);

      void update(const resources_3d::TrackPath* track_path);
      void update(const resources_3d::TrackPath* track_path, 
                  std::size_t node_index, std::size_t node_count);

    private:
      struct TextureMapping;
      struct UniformLocations;
      const TextureMapping* find_terrain_texture(std::uint16_t texture_id) const;
      void load_terrain_textures(const resources_3d::TextureLibrary& texture_library);
      UniformLocations load_uniform_locations() const;

      struct TextureMapping
      {
        std::uint16_t texture_id;
        std::uint32_t texture_size;
        std::size_t texture_array_index;
        std::size_t texture_layer_index;
      };

      struct RenderComponent
      {
        std::size_t texture_array_index;
        std::size_t element_index;
        std::size_t element_count;

        GLuint vertex_buffer;
        GLuint index_buffer;

        const void* user_data = nullptr;
      };

      struct UniformLocations
      {
        GLuint projection_matrix;
        GLuint view_matrix;
        GLuint texture_sampler;
      };

      struct TrackPath
      {
        const resources_3d::TrackPath* track_path;

        graphics::Buffer vertex_buffer;
        graphics::Buffer index_buffer;
      };

      graphics::ShaderProgram basic_shader_;

      UniformLocations uniform_locations_;

      graphics::Sampler texture_sampler_;
      graphics::Sampler height_map_sampler_;

      graphics::Texture height_map_texture_;
      std::vector<graphics::TextureArray> textures_;

      float height_map_max_z_ = 0.0f;
      Vector2f height_map_cell_size_;

      graphics::Buffer basic_vertices_;
      graphics::Buffer basic_indices_;

      std::vector<RenderComponent> render_components_;      
      std::vector<TextureMapping> texture_mapping_;
      std::unordered_map<const resources_3d::TrackPath*, TrackPath> track_paths_;

      std::vector<TerrainVertex> vertex_cache_;
      std::vector<GLuint> index_cache_;
      std::vector<PathVertexPoint<const resources_3d::TrackPathNode*>> path_vertex_point_cache_;
    };
  }
}

#endif