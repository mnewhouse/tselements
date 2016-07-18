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
#include "model_3d.hpp"

#include "utility/color.hpp"
#include "utility/vector2.hpp"
#include "utility/vector3.hpp"
#include "utility/triangle_utilities.hpp"

#include <glm/mat4x4.hpp>

#include <boost/optional.hpp>

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
    class HeightMap;

    struct TrackPath;
    struct TrackPathNode;    
  }

  namespace scene_3d
  {
    struct TerrainVertex
    {
      Vector3f position;
      Vector3f tex_coords;
      Vector3<std::int8_t> normal;
      Colorb color;
    };

    class TerrainScene
    {
    public:
      TerrainScene();

      void load_track_terrains(const resources_3d::Track& track);
      void rebuild_track_terrains(const resources_3d::Track& track);

      void rebuild_height_map(const resources_3d::HeightMap& height_map, IntRect updated_area);

      const graphics::Texture& height_map_texture() const;
      float height_map_max_z() const;
      Vector2f height_map_cell_size() const;

      void render(const glm::mat4x4& view_mat, const glm::mat4x4& projection_mat) const;

      void register_track_path(const resources_3d::TrackPath* track_path, const resources_3d::HeightMap& height_map);

      void update(const resources_3d::TrackPath* track_path, const resources_3d::HeightMap& height_map);
      void update(const resources_3d::TrackPath* track_path, 
                  std::size_t node_index, std::size_t node_count);

      boost::optional<Vector3f> find_terrain_position_at(Vector2i absolute_position, Vector2i screen_size,
                                                         IntRect view_port, const resources_3d::HeightMap& height_map,
                                                         const glm::mat4& projected_view) const;

    private:
      struct TextureMapping;
      struct UniformLocations;
      const TextureMapping* find_terrain_texture(std::uint16_t texture_id) const;
      void load_terrain_textures(const resources_3d::TextureLibrary& texture_library);
      UniformLocations load_uniform_locations() const;

      struct TextureMapping;
      resources_3d::BasicModel<TerrainVertex> 
        generate_base_terrain_model(const resources_3d::HeightMap& height_map, Vector3u world_size, 
                                    const TextureMapping& base_texture);

      void build_base_terrain(const resources_3d::Track& track);

      void generate_height_map_texture(const resources_3d::Track& track);
      void update_height_map_uniforms();

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

        enum Type
        {
          Default, Path
        };

        Type type = Default;
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

        resources_3d::BasicModel<TerrainVertex> model;
      };

      graphics::ShaderProgram basic_shader_;
      graphics::Sampler texture_sampler_;
      graphics::Sampler height_map_sampler_;

      graphics::Texture height_map_texture_;
      float height_map_max_z_ = 0.0f;
      Vector2f height_map_cell_size_;

      UniformLocations uniform_locations_;
      std::vector<graphics::TextureArray> textures_;

      graphics::Buffer basic_vertices_;
      graphics::Buffer basic_indices_;

      Vector3u track_size_;

      std::vector<RenderComponent> render_components_;      
      std::vector<TextureMapping> texture_mapping_;
      std::unordered_map<const resources_3d::TrackPath*, TrackPath> track_paths_;

      std::vector<PathVertexPoint> path_vertex_point_cache_;
    };

    template <typename VertexType>
    void align_model_faces_to_terrain_grid(resources_3d::BasicModel<VertexType>& model, 
                                           std::uint32_t cell_size)
    {
      auto real_cell_size = static_cast<float>(cell_size);

      std::vector<resources_3d::ModelFace> new_faces;

      // Loop through all the faces
      for (std::size_t face_index = 0; face_index < model.faces.size(); )
      {
        // If the face is contained in more than one cell, split it into 2-6 faces depending on the
        // shape of the cell/face intersection. If the face is contained in just one cell, do nothing 
        // and advance the loop.
        const auto& face = model.faces[face_index];
        auto face_vertices = resources_3d::face_vertices(model, face);

        auto face_triangle = make_triangle(
          Vector2f(face_vertices.first.position.x, face_vertices.first.position.y),
          Vector2f(face_vertices.second.position.x, face_vertices.second.position.y),
          Vector2f(face_vertices.third.position.x, face_vertices.third.position.y)
          );

        auto bounding_box = make_rect_from_points({ face_triangle[0], face_triangle[1], face_triangle[2] });
        auto grid_left = static_cast<std::int32_t>(bounding_box.left) / cell_size;
        auto grid_top = static_cast<std::int32_t>(bounding_box.top) / cell_size;
        auto grid_right = (static_cast<std::int32_t>(bounding_box.right()) + cell_size - 1) / cell_size;
        auto grid_bottom = (static_cast<std::int32_t>(bounding_box.bottom()) + cell_size - 1) / cell_size;
        IntRect grid_bounds(grid_left, grid_top, grid_right - grid_left, grid_bottom - grid_top);

        if (grid_bounds.width * grid_bounds.height >= 2)
        {
          // Now, loop through all cells 
          for (auto cell_y = grid_top; cell_y < grid_bottom; ++cell_y)
          {
            for (auto cell_x = grid_left; cell_x < grid_right; ++cell_x)
            {
              // If the cell is fully contained in the face, just add
            }
          }
        }

        else
        {
          ++face_index;
        }
      }
    }
  }
}

#endif