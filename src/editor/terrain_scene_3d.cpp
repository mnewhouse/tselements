/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "terrain_scene_3d.hpp"
#include "track_3d.hpp"
#include "scene_3d_shaders.hpp"
#include "height_map_shaders.hpp"
#include "track_path.hpp"
#include "path_vertices_detail.hpp"

#include "graphics/image_loader.hpp"
#include "graphics/gl_check.hpp"

#include "utility/color.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <algorithm>

namespace ts
{
  namespace scene_3d
  {
    namespace detail
    {
      auto create_terrain_shader_program()
      {
        const char* vertex_shaders[] = { version_string, height_map_vertex_shader_functions, terrain_vertex_shader };
        const char* fragment_shaders[] = { version_string, terrain_fragment_shader };

        return graphics::create_shader_program(vertex_shaders, fragment_shaders);
      }

      template <typename TextureEntryIt>
      graphics::TextureArray load_texture_array(TextureEntryIt it, TextureEntryIt end, std::uint32_t texture_size,
                                                graphics::DefaultImageLoader& image_loader)
      {
        auto texture_count = static_cast<GLuint>(std::distance(it, end));
        if (texture_count != 0)
        {
          GLuint texture_name = 0;
          glGenTextures(1, &texture_name);

          Vector3u dimensions = { texture_size, texture_size, texture_count };
          graphics::TextureArray result(texture_name, dimensions);

          GLuint mipmap_level_count = 0;
          for (auto n = texture_size; n >>= 1; ) ++mipmap_level_count;

          glCheck(glBindTexture(GL_TEXTURE_2D_ARRAY, texture_name));
          glCheck(glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, texture_size, texture_size,
                                 texture_count));

          for (GLuint z_offset = 0; it != end; ++it, ++z_offset)
          {
            const resources_3d::TextureEntry& entry = *it;
            const auto& image = image_loader.load_image(entry.image_file);
            const auto& rect = entry.image_rect;

            if (rect.left == 0 && rect.top == 0 &&
                image.getSize().x == rect.width && image.getSize().y == rect.height)
            {
              glCheck(glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, z_offset,
                                      dimensions.x, dimensions.y, 1,
                                      GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr()));
            }

            else
            {
              sf::Image temp_image;
              temp_image.create(rect.width, rect.height);
              temp_image.copy(image, 0, 0, { rect.left, rect.top, rect.width, rect.height });
              glCheck(glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, z_offset, 
                                      dimensions.x, dimensions.y, 1,
                                      GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr()));
            }
          }

          return result;
        }

        return{};
      }

      auto create_height_map_texture(const resources_3d::HeightMap& height_map,
                                     Vector3u world_size)
      {
        auto cell_size = height_map.cell_size();
        auto size = make_vector2((world_size.x + cell_size + 1) / cell_size,
                                 (world_size.y + cell_size + 1) / cell_size);

        auto texture_size = make_vector2(graphics::next_power_of_two(size.x),
                                         graphics::next_power_of_two(size.y));

        auto max_x = std::min<std::size_t>(height_map.size().x, texture_size.x);
        auto max_y = std::min<std::size_t>(height_map.size().y, texture_size.y);

        std::vector<std::uint8_t> height_data(texture_size.x * texture_size.y);
        for (std::uint32_t y = 0; y != max_y; ++y)
        {
          auto idx = y * texture_size.x;
          for (std::uint32_t x = 0; x != max_x; ++x, ++idx)
          {
            height_data[idx] = static_cast<std::uint8_t>(height_map(x, y) / world_size.z * 255.0f);
          }
        }

        GLuint height_map_texture;
        glCheck(glGenTextures(1, &height_map_texture));
        glCheck(glBindTexture(GL_TEXTURE_2D, height_map_texture));
        glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, texture_size.x, texture_size.y, 0,
                             GL_ALPHA, GL_UNSIGNED_BYTE, height_data.data()));

        return graphics::Texture(height_map_texture, vector2_cast<std::uint32_t>(texture_size));
      }

      auto generate_base_terrain_vertices(Vector2u world_size, std::uint32_t cell_size,
                                          std::size_t base_terrain_id, std::uint32_t base_texture_size)
      {
        auto num_columns = (world_size.x + cell_size - 1) / cell_size;
        auto num_rows = (world_size.y + cell_size - 1) / cell_size;

        std::vector<TerrainVertex> vertices(num_rows * num_columns);
        std::vector<GLuint> indices;

        auto tex_coord_multiplier = 1.0f / (base_texture_size * 0.5f);
        auto make_vertex = [=](auto coords)
        {
          TerrainVertex vertex;
          vertex.position.x = static_cast<float>(coords.x);
          vertex.position.y = static_cast<float>(coords.y);
          vertex.position.z = 0.0f;

          vertex.tex_coords =
          {
            vertex.position.x * tex_coord_multiplier,
            vertex.position.y * tex_coord_multiplier,
            0.0f
          };

          vertex.normal = { 0, 0, 127 };
          return vertex;
        };
       
        for (std::uint32_t y = 0; y < num_rows; ++y)
        {
          auto idx = y * num_columns;
          auto y_coord = std::min(y * cell_size, world_size.y);

          for (std::uint32_t x = 0; x < num_columns; ++x)
          {
            auto coords = make_vector2(std::min(x * cell_size, world_size.x), y_coord);
            vertices[idx] = make_vertex(coords);
            ++idx;
          }
        }

        for (std::uint32_t y = 0; y != num_rows - 1; ++y)
        {
          GLuint index = y * num_columns;

          for (std::uint32_t x = 0; x != num_columns - 1; ++x, ++index)
          {
            indices.push_back(index);
            indices.push_back(index + 1);
            indices.push_back(index + num_columns);
            indices.push_back(index + 1);
            indices.push_back(index + num_columns);
            indices.push_back(index + 1 + num_columns);
          }
        }

        return std::make_pair(std::move(vertices), std::move(indices));
      }

      auto create_texture_sampler()
      {
        GLuint sampler;
        glCheck(glCreateSamplers(1, &sampler));

        glCheck(glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        glCheck(glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

        return graphics::Sampler(sampler);
      }
    }

    TerrainScene::TerrainScene()
      : basic_shader_(detail::create_terrain_shader_program()),
        uniform_locations_(load_uniform_locations()),
        texture_sampler_(detail::create_texture_sampler()),
        height_map_sampler_(create_height_map_sampler())
    {
      basic_vertices_ = graphics::create_buffer();
      basic_indices_ = graphics::create_buffer();
    }

    const TerrainScene::TextureMapping* 
      TerrainScene::find_terrain_texture(std::uint16_t texture_id) const
    {
      auto it = std::lower_bound(texture_mapping_.begin(), texture_mapping_.end(), texture_id,
                                 [](const auto& entry, std::uint16_t texture_id)
      {
        return entry.texture_id < texture_id;
      });

      if (it != texture_mapping_.end() && it->texture_id == texture_id)
      {
        return &*it;
      }

      return nullptr;
    }

    void TerrainScene::load_track_terrains(const resources_3d::Track& track)
    {
      load_terrain_textures(track.texture_library());

      auto base_texture = find_terrain_texture(track.base_texture());
      if (base_texture == nullptr)
      {
        throw std::logic_error("track does not have a valid base texture id");
      }

      const auto& height_map = track.height_map();
      auto cell_size = height_map.cell_size();
      auto world_size = track.size();
      auto world_size_2d = track.size_2d();
      height_map_texture_ = detail::create_height_map_texture(height_map, world_size);
      auto texture_size = height_map_texture_.size();

      height_map_max_z_ = static_cast<float>(world_size.z);
      height_map_cell_size_ =
      {
        1.0f / (cell_size * texture_size.x),
        1.0f / (cell_size * texture_size.y)
      };

      auto height_map_uniforms = get_height_map_uniform_locations(basic_shader_);
      glCheck(glUseProgram(basic_shader_.get()));      
      glCheck(glUniform1f(height_map_uniforms.height_map_max_z, height_map_max_z_));
      glCheck(glUniform2f(height_map_uniforms.height_map_cell_size,
                          height_map_cell_size_.x,
                          height_map_cell_size_.y));

      glCheck(glUseProgram(0));

      auto buffer_contents = detail::generate_base_terrain_vertices(world_size_2d,
                                                                    height_map.cell_size(),
                                                                    base_texture->texture_id,
                                                                    base_texture->texture_size);


      auto& basic_vertices = buffer_contents.first;
      auto& basic_indices = buffer_contents.second;

      RenderComponent component;
      component.vertex_buffer = basic_vertices_.get();
      component.index_buffer = basic_indices_.get();
      component.element_index = 0;
      component.element_count = buffer_contents.second.size();
      component.texture_array_index = base_texture->texture_array_index;
      render_components_.push_back(component);

      glCheck(glBindBuffer(GL_ARRAY_BUFFER, basic_vertices_.get()));
      glCheck(glBufferData(GL_ARRAY_BUFFER, basic_vertices.size() * sizeof(TerrainVertex),
                   basic_vertices.data(), GL_STATIC_DRAW));

      glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, basic_indices_.get()));
      glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER, basic_indices.size() * sizeof(GLuint),
                           basic_indices.data(), GL_STATIC_DRAW));

      glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
      glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

      for (auto path : track.paths())
      {
        register_track_path(path);
      }
    }

    void TerrainScene::load_terrain_textures(const resources_3d::TextureLibrary& texture_library)
    {
      // Read all of the texture library's textures into an array
      // and sort it by texture size.
      auto terrain_textures = texture_library.textures();
      std::stable_sort(terrain_textures.begin(), terrain_textures.end(),
                       [](const auto& a, const auto& b)
      {
        return a.image_rect.width < b.image_rect.width;
      });

      graphics::DefaultImageLoader image_loader;

      std::size_t texture_array_index = 0;
      auto texture_it = terrain_textures.begin();
      for (; texture_it != terrain_textures.end(); )
      {
        auto size = texture_it->image_rect.width;
        auto end_it = std::find_if(std::next(texture_it), terrain_textures.end(),
                                   [size](const auto& entry)
        {
          return entry.image_rect.width != size;
        });

        textures_.push_back(detail::load_texture_array(texture_it, end_it, size, image_loader));

        std::size_t texture_layer_index = 0;
        for (auto it = texture_it; it != end_it; ++it, ++texture_layer_index)
        {
          TextureMapping mapping;
          mapping.texture_id = it->id;
          mapping.texture_array_index = texture_array_index;
          mapping.texture_layer_index = texture_layer_index;
          mapping.texture_size = texture_it->image_rect.width;
          texture_mapping_.push_back(mapping);
        }

        texture_it = end_it;
        ++texture_array_index;
      }

      std::sort(texture_mapping_.begin(), texture_mapping_.end(),
                [](const auto& a, const auto& b)
      {
        return a.texture_id < b.texture_id;
      });
    }

    void TerrainScene::render(const glm::mat4x4& view_matrix, const glm::mat4x4& projection_matrix) const
    {
      glCheck(glEnable(GL_MULTISAMPLE));
      glCheck(glEnable(GL_BLEND));
      glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

      auto basic_shader = basic_shader_.get();

      glCheck(glBindSampler(0, texture_sampler_.get()));
      glCheck(glBindSampler(1, height_map_sampler_.get()));

      glCheck(glActiveTexture(GL_TEXTURE1));
      glCheck(glBindTexture(GL_TEXTURE_2D, height_map_texture_.get()));

      glCheck(glActiveTexture(GL_TEXTURE0));

      glCheck(glUseProgram(basic_shader));
      glCheck(glUniformMatrix4fv(uniform_locations_.projection_matrix, 1, GL_FALSE,
                                 glm::value_ptr(projection_matrix)));

      glCheck(glUniformMatrix4fv(uniform_locations_.view_matrix, 1, GL_FALSE,
                                 glm::value_ptr(view_matrix)));

      glCheck(glUniform1i(uniform_locations_.texture_sampler, 0));

      auto height_map_uniforms = get_height_map_uniform_locations(basic_shader_);
      glCheck(glUniform1i(height_map_uniforms.height_map_sampler, 1));

      // For every terrain component... (base terrain, custom vertices, paths)
      //   Select the proper shader and draw it.

      for (const auto& component : render_components_)
      {
        if (component.texture_array_index < textures_.size())
        {
          const auto& texture = textures_[component.texture_array_index];
          glCheck(glBindTexture(GL_TEXTURE_2D_ARRAY, texture.get()));
        }

        else
        {
          // Todo: bind dummy texture?
          glCheck(glBindTexture(GL_TEXTURE_2D_ARRAY, 0));
        }

        glBindBuffer(GL_ARRAY_BUFFER, component.vertex_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, component.index_buffer);

        glCheck(glEnableVertexAttribArray(0));
        glCheck(glEnableVertexAttribArray(1));
        glCheck(glEnableVertexAttribArray(2));

        glCheck(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex),
                                      reinterpret_cast<const void*>(offsetof(TerrainVertex, position))));

        glCheck(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex),
                                      reinterpret_cast<const void*>(offsetof(TerrainVertex, tex_coords))));
        glCheck(glVertexAttribPointer(2, 3, GL_BYTE, GL_TRUE, sizeof(TerrainVertex),
                                      reinterpret_cast<const void*>(offsetof(TerrainVertex, normal))));
        
        auto offset = component.element_index * sizeof(GLuint);
        glCheck(glDrawElements(GL_TRIANGLES, component.element_count, GL_UNSIGNED_INT,
                       reinterpret_cast<const void*>(offset)));

        glCheck(glDisableVertexAttribArray(0));
        glCheck(glDisableVertexAttribArray(1));
        glCheck(glDisableVertexAttribArray(2));
      }
    }

    TerrainScene::UniformLocations TerrainScene::load_uniform_locations() const
    {
      auto program = basic_shader_.get();

      UniformLocations locations;
      locations.projection_matrix = glCheck(glGetUniformLocation(program, "u_projectionMatrix"));
      locations.view_matrix = glCheck(glGetUniformLocation(program, "u_viewMatrix"));
      locations.texture_sampler = glCheck(glGetUniformLocation(program, "u_textureSampler"));

      return locations;
    }

    const graphics::Texture& TerrainScene::height_map_texture() const
    {
      return height_map_texture_;
    }

    Vector2f TerrainScene::height_map_cell_size() const
    {
      return height_map_cell_size_;
    }

    float TerrainScene::height_map_max_z() const
    {
      return height_map_max_z_;
    }

    void TerrainScene::register_track_path(const resources_3d::TrackPath* track_path)
    {
      TrackPath entry;
      entry.track_path = track_path;
      entry.vertex_buffer = graphics::create_buffer();
      entry.index_buffer = graphics::create_buffer();

      track_paths_.insert(std::make_pair(track_path, std::move(entry)));

      update(track_path);
    }

    void TerrainScene::update(const resources_3d::TrackPath* path)
    {
      // Update the visible components of a track path.
      // We need to add a render component for all stroke styles,
      // and rebuild the dirty vertices.

      auto path_it = track_paths_.find(path);
      if (path_it != track_paths_.end())
      {
        auto& path_data = path_it->second;
        auto user_data = static_cast<const void*>(&path_data);

        // First, remove all render components that were previously in use by this track path.
        auto component_end = std::remove_if(render_components_.begin(), render_components_.end(),
                                            [=](const RenderComponent& component)
        {
          return component.user_data == user_data;
        });

        render_components_.erase(component_end, render_components_.end());

        // Precompute the vertex points
        path_vertex_point_cache_.clear();
        vertex_cache_.clear();
        index_cache_.clear();

        compute_path_vertex_points(path->nodes.data(), path->nodes.data() + path->nodes.size(),
                                   0.04f, path_vertex_point_cache_);

        // Then, for every stroke style, generate a render component.
        for (const auto& stroke_properties : path->strokes)
        {
          auto texture_mapping = find_terrain_texture(stroke_properties.texture_id);
          auto texture_size = 1.0f;
          auto texture_z = 0.0f;
          auto texture_array_index = std::numeric_limits<std::size_t>::max();
          if (texture_mapping)
          {
            texture_size = static_cast<float>(texture_mapping->texture_size);
            texture_z = static_cast<float>(texture_mapping->texture_layer_index);
            texture_array_index = texture_mapping->texture_array_index;
          }

          auto color = stroke_properties.color;
          
          auto vertex_func = [=](Vector3f position, Vector2f tex_coord, Vector3f normal)
          {
            normal = normalize(normal * normal * normal);

            TerrainVertex vertex;
            vertex.position = position;
            vertex.color = color;
            vertex.normal = vector3_cast<std::int8_t>(normal * 127.0f);
            vertex.tex_coords =
            {
              tex_coord.x / texture_size,
              tex_coord.y / texture_size,
              texture_z
            };

            return vertex;
          };

          RenderComponent component;
          component.vertex_buffer = path_data.vertex_buffer.get();
          component.index_buffer = path_data.index_buffer.get();
          component.element_index = index_cache_.size();
          component.user_data = user_data;

          // Now, genenerate the path vertices
          generate_path_vertices(path_vertex_point_cache_, stroke_properties,
                                 vertex_func, std::back_inserter(vertex_cache_),
                                 vertex_cache_.size(), std::back_inserter(index_cache_));

          component.element_count = index_cache_.size() - component.element_index;
          component.texture_array_index = texture_array_index;

          // And add the render component.
          render_components_.push_back(component);          
        }

        // Finally, upload the buffer data.
        glCheck(glBindBuffer(GL_ARRAY_BUFFER, path_it->second.vertex_buffer.get()));
        glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, path_it->second.index_buffer.get()));

        glCheck(glBufferData(GL_ARRAY_BUFFER, vertex_cache_.size() * sizeof(TerrainVertex),
                             vertex_cache_.data(), GL_STATIC_DRAW));

        glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_cache_.size() * sizeof(GLuint),
                             index_cache_.data(), GL_STATIC_DRAW));
      }      
    }

    void TerrainScene::update(const resources_3d::TrackPath* path, 
                              std::size_t node_index, std::size_t node_count)
    {
      // Only update the path nodes as specified in the function arguments.
      // We can use this to rebuild the vertex buffers more efficiently.

      update(path);
    }
  }
}