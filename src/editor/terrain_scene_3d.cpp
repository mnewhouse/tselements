/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifdef _MSC_VER
#pragma warning(disable : 4503)
#endif

#include "stdinc.hpp"

#include "terrain_scene_3d.hpp"
#include "track_3d.hpp"
#include "scene_3d_shaders.hpp"
#include "track_path.hpp"
#include "path_vertices_detail.hpp"
#include "terrain_model_3d_detail.hpp"
#include "height_map_shaders.hpp"

#include "graphics/image_loader.hpp"
#include "graphics/gl_check.hpp"

#include "utility/color.hpp"
#include "utility/line_intersection.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GL/glew.h>

#include <algorithm>

namespace ts
{
  namespace scene_3d
  {
    namespace detail
    {
      auto create_terrain_shader_program()
      {
        const char* vertex_shaders[] = 
        { 
          version_string, 
          height_map_vertex_shader_functions,
          terrain_vertex_shader 
        };

        const char* fragment_shaders[] = 
        { 
          version_string,
          terrain_fragment_shader };

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

      auto create_texture_sampler()
      {
        GLuint sampler;
        glCheck(glCreateSamplers(1, &sampler));

        glCheck(glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        glCheck(glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

        return graphics::Sampler(sampler);
      }

      auto create_height_map_texture_data(const resources_3d::HeightMap& height_map,
                                          IntRect area, float world_height)
      {
        std::vector<std::uint8_t> height_data(area.width * area.height);
        auto max_x = std::min(static_cast<std::int32_t>(height_map.size().x), area.right());
        auto max_y = std::min(static_cast<std::int32_t>(height_map.size().y), area.bottom());

        for (auto y = area.top; y < max_y; ++y)
        {
          auto idx = (y - area.top) * area.width;
          for (auto x = 0; x < max_x; ++x, ++idx)
          {
            height_data[idx] = static_cast<std::uint8_t>(height_map(x, y) / world_height * 255.0f);
          }
        }

        return height_data;
      }

      auto create_height_map_texture(const resources_3d::HeightMap& height_map,
                                     Vector3u world_size)
      {
        auto cell_size = height_map.cell_size();
        auto size = make_vector2((world_size.x + cell_size - 1) / cell_size,
                                 (world_size.y + cell_size - 1) / cell_size);

        auto texture_size = make_vector2(graphics::next_power_of_two(size.x),
                                         graphics::next_power_of_two(size.y));

        IntRect area;
        area.left = 0;
        area.top = 0;
        area.width = static_cast<std::int32_t>(texture_size.x);
        area.height = static_cast<std::int32_t>(texture_size.y);

        auto height_data = create_height_map_texture_data(height_map, area,
                                                          static_cast<float>(world_size.z));


        GLuint height_map_texture;
        glCheck(glGenTextures(1, &height_map_texture));
        glCheck(glBindTexture(GL_TEXTURE_2D, height_map_texture));

        glCheck(glPixelStorei(GL_PACK_ALIGNMENT, 1));
        glCheck(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
        glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, texture_size.x, texture_size.y, 0,
                             GL_ALPHA, GL_UNSIGNED_BYTE, height_data.data()));

        glCheck(glPixelStorei(GL_UNPACK_ALIGNMENT, 4));
        glCheck(glPixelStorei(GL_PACK_ALIGNMENT, 4));

        return graphics::Texture(height_map_texture, vector2_cast<std::uint32_t>(texture_size));
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

    resources_3d::BasicModel<TerrainVertex> 
      TerrainScene::generate_base_terrain_model(const resources_3d::HeightMap& height_map,
                                                Vector3u world_size, const TextureMapping& texture_mapping)
    {
      auto cell_size = height_map.cell_size();

      auto texture_z = static_cast<float>(texture_mapping.texture_layer_index);
      auto texture_scale = 1.0f / texture_mapping.texture_size;

      auto make_vertex = [&](Vector2f position)
      {
        auto z = interpolate_height_at(height_map, position);

        TerrainVertex vertex;
        vertex.position = make_3d(position, z);
        vertex.color = { 255, 255, 255, 255 };
        vertex.tex_coords.x = position.x * 2.0f * texture_scale;
        vertex.tex_coords.y = position.y * 2.0f * texture_scale;
        vertex.tex_coords.z = texture_z;
        return vertex;
      };

      auto bounds = vector3_cast<float>(world_size);          
      auto num_columns = (world_size.x + cell_size - 1) / cell_size;
      auto num_rows = (world_size.y + cell_size - 1) / cell_size;
      auto real_cell_size = static_cast<float>(cell_size);

      resources_3d::BasicModel<TerrainVertex> model;
      auto& vertices = model.vertices;
      auto& faces = model.faces;
      vertices.resize(num_rows * num_columns);

      for (std::uint32_t map_y = 0; map_y < num_rows; ++map_y)
      {
        auto idx = map_y * num_columns;
        auto y = map_y * real_cell_size;

        for (std::uint32_t map_x = 0; map_x < num_columns; ++map_x, ++idx)
        {
          vertices[idx] = make_vertex({ map_x * real_cell_size, y });
        }
      }

      auto quad_rows = num_rows - 1;
      auto quad_columns = num_columns - 1;

      for (std::uint32_t y = 0; y != quad_rows; ++y)
      {
        auto index = y * num_columns;

        for (std::uint32_t x = 0; x != quad_columns; ++x, ++index)
        {
          faces.push_back({ index, index + 1, index + num_columns });
          faces.push_back({ index + 1, index + num_columns, index + 1 + num_columns });
        }
      }

      return model;
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

      build_base_terrain(track);

      generate_height_map_texture(track);

      for (auto path : track.paths())
      {
        register_track_path(path, track.height_map());
      }    
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

    void TerrainScene::generate_height_map_texture(const resources_3d::Track& track)
    {
      const auto& height_map = track.height_map();
      auto cell_size = height_map.cell_size();

      height_map_texture_ = detail::create_height_map_texture(height_map, track.size());
      auto texture_size = height_map_texture_.size();

      height_map_max_z_ = static_cast<float>(track.size().z);
      height_map_cell_size_ =
      {
        1.0f / (cell_size * texture_size.x),
        1.0f / (cell_size * texture_size.y)
      };

      update_height_map_uniforms();
    }

    void TerrainScene::rebuild_height_map(const resources_3d::HeightMap& height_map, IntRect area)
    {
      height_map_texture_ = detail::create_height_map_texture(height_map, track_size_);

      /*
      auto height_data = detail::create_height_map_texture_data(height_map, area,
                                                                static_cast<float>(track_size_.z));

      glCheck(glPixelStorei(GL_PACK_ALIGNMENT, 1));
      glCheck(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
      glCheck(glBindTexture(GL_TEXTURE_2D, height_map_texture_.get()));
      glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, area.left, area.top, area.width, area.height,
                              GL_ALPHA, GL_UNSIGNED_BYTE, height_data.data()));

      glCheck(glPixelStorei(GL_UNPACK_ALIGNMENT, 4));
      glCheck(glPixelStorei(GL_PACK_ALIGNMENT, 4));
      */
    }

    void TerrainScene::update_height_map_uniforms()
    {
      auto height_map_uniforms = get_height_map_uniform_locations(basic_shader_);
      glCheck(glUseProgram(basic_shader_.get()));
      glCheck(glUniform1f(height_map_uniforms.height_map_max_z, height_map_max_z_));
      glCheck(glUniform2f(height_map_uniforms.height_map_cell_size,
                          height_map_cell_size_.x,
                          height_map_cell_size_.y));
      glCheck(glUniform1i(height_map_uniforms.height_map_sampler, 1));

      glCheck(glUseProgram(0));
    }

    void TerrainScene::build_base_terrain(const resources_3d::Track& track)
    {
      const auto& height_map = track.height_map();

      auto base_texture = find_terrain_texture(track.base_texture());
      if (base_texture == nullptr)
      {
        throw std::logic_error("track does not have a valid base texture id");
      }

      track_size_ = track.size();
      auto base_terrain_model = generate_base_terrain_model(height_map, track_size_, 
                                                            *base_texture);

      const auto& vertices = base_terrain_model.vertices;
      const auto& faces = base_terrain_model.faces;

      RenderComponent component;
      component.vertex_buffer = basic_vertices_.get();
      component.index_buffer = basic_indices_.get();
      component.element_index = 0;
      component.element_count = faces.size() * 3;
      component.texture_array_index = base_texture->texture_array_index;
      component.user_data = nullptr;
      render_components_.push_back(component);

      glCheck(glBindBuffer(GL_ARRAY_BUFFER, basic_vertices_.get()));
      glCheck(glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices.front()),
                           vertices.data(), GL_STATIC_DRAW));

      glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, basic_indices_.get()));
      glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(faces.front()),
                           faces.data(), GL_STATIC_DRAW));

      glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
      glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
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
      glCheck(glEnable(GL_DEPTH_TEST));
      glCheck(glDisable(GL_STENCIL_TEST));
      glCheck(glDepthMask(GL_TRUE));
      glCheck(glDepthFunc(GL_LEQUAL));

      auto basic_shader = basic_shader_.get();

      glCheck(glBindSampler(1, height_map_sampler_.get()));
      glCheck(glActiveTexture(GL_TEXTURE1));
      glCheck(glBindTexture(GL_TEXTURE_2D, height_map_texture_.get()));

      glCheck(glBindSampler(0, texture_sampler_.get()));
      glCheck(glActiveTexture(GL_TEXTURE0));

      glCheck(glUseProgram(basic_shader));
      glCheck(glUniformMatrix4fv(uniform_locations_.projection_matrix, 1, GL_FALSE,
                                 glm::value_ptr(projection_matrix)));

      glCheck(glUniformMatrix4fv(uniform_locations_.view_matrix, 1, GL_FALSE,
                                 glm::value_ptr(view_matrix)));

      glCheck(glUniform1i(uniform_locations_.texture_sampler, 0));

      // For every terrain component... (base terrain, custom vertices, paths)
      // Select the appropriate shader and draw the geometry.

      glCheck(glClear(GL_STENCIL_BUFFER_BIT));
      glCheck(glDisable(GL_STENCIL_TEST));      
      glCheck(glStencilOp(GL_KEEP, GL_KEEP, GL_INCR));
      glCheck(glStencilMask(0xFF));

      for (const auto& component : render_components_)
      {
        if (&component == &render_components_[1])
        {
          glCheck(glDisable(GL_DEPTH_TEST));
          glCheck(glDepthMask(GL_FALSE));
          glCheck(glEnable(GL_STENCIL_TEST));
          glCheck(glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP));
          glCheck(glStencilFunc(GL_GREATER, 1, 0xFF));
        }

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
        glCheck(glEnableVertexAttribArray(3));

        using Vertex = TerrainVertex;
        glCheck(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                      reinterpret_cast<const void*>(offsetof(Vertex, position))));

        glCheck(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                      reinterpret_cast<const void*>(offsetof(Vertex, tex_coords))));

        glCheck(glVertexAttribPointer(2, 3, GL_BYTE, GL_TRUE, sizeof(Vertex),
                                      reinterpret_cast<const void*>(offsetof(Vertex, normal))));

        glCheck(glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex),
                                      reinterpret_cast<const void*>(offsetof(Vertex, color))));
        
        auto offset = component.element_index * sizeof(std::uint32_t);
        glCheck(glDrawElements(GL_TRIANGLES, component.element_count, GL_UNSIGNED_INT,
                               reinterpret_cast<const void*>(offset)));

        glCheck(glDisableVertexAttribArray(0));
        glCheck(glDisableVertexAttribArray(1));
        glCheck(glDisableVertexAttribArray(2));
        glCheck(glDisableVertexAttribArray(3));
      }

      glCheck(glDepthMask(GL_TRUE));
      glCheck(glDisable(GL_STENCIL_TEST));
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

    void TerrainScene::register_track_path(const resources_3d::TrackPath* track_path,
                                           const resources_3d::HeightMap& height_map)
    {
      TrackPath entry;
      entry.track_path = track_path;
      entry.vertex_buffer = graphics::create_buffer();
      entry.index_buffer = graphics::create_buffer();

      track_paths_.insert(std::make_pair(track_path, std::move(entry)));

      update(track_path, height_map);
    }



    void TerrainScene::update(const resources_3d::TrackPath* path, const resources_3d::HeightMap& height_map)
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

        path_vertex_point_cache_.clear();

        // Precompute the vertex points
        compute_path_vertex_points(path->nodes.begin(), path->nodes.end(),
                                   0.05f, path_vertex_point_cache_);

        auto component_index = render_components_.size();

        auto& model = path_it->second.model;
        model.faces.clear();
        model.lines.clear();
        model.vertices.clear();

        // Then, for every stroke style, generate a render component.
        for (const auto& stroke : path->strokes)
        {
          const auto& stroke_properties = stroke.properties;
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

          RenderComponent component;
          component.vertex_buffer = path_data.vertex_buffer.get();
          component.index_buffer = path_data.index_buffer.get();
          component.element_index = model.faces.size() * 3;
          component.user_data = user_data;

          scene_3d::PathCellAlignment cell_alignment;
          cell_alignment.align = true;
          cell_alignment.cell_size = 16;
          cell_alignment.triangle_orientation = scene_3d::PathCellAlignment::TopLeft;

          auto transform_vertex = [&](const scene_3d::PathVertex& path_vertex)
          {
            auto z = interpolate_height_at(height_map, path_vertex.position);

            TerrainVertex vertex;
            vertex.position = make_3d(path_vertex.position, 0.0f);
            vertex.tex_coords = path_vertex.tex_coords;
            vertex.normal = vector3_cast<std::int8_t>(path_vertex.normal * 127.0f);
            vertex.color = path_vertex.color;
            return vertex;
          };

          // Now, genenerate the path vertices
          generate_path_vertices2(*path, stroke.properties, path_vertex_point_cache_,
                                  texture_size, texture_z, cell_alignment,
                                  model, transform_vertex);


          component.element_count = model.faces.size() * 3 - component.element_index;
          component.texture_array_index = texture_array_index;          

          // And add the render component.
          render_components_.push_back(component);          
        }

        // Finally, upload the buffer data.
        glCheck(glBindBuffer(GL_ARRAY_BUFFER, path_it->second.vertex_buffer.get()));
        glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, path_it->second.index_buffer.get()));

        glCheck(glBufferData(GL_ARRAY_BUFFER, model.vertices.size() * sizeof(model.vertices.front()),
                             model.vertices.data(), GL_STATIC_DRAW));

        glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.faces.size() * sizeof(model.faces.front()),
                             model.faces.data(), GL_STATIC_DRAW));
      }      
    }

    namespace detail
    {
      template <typename Triangle>
      boost::optional<std::pair<float, Vector3f>>
        triangle_ray_intersection(const Triangle& triangle,
                                  glm::vec3 ray_start, glm::vec3 ray_end)
      {
        auto u = triangle[1] - triangle[0];
        auto v = triangle[2] - triangle[0];
        auto normal = normalize(glm::cross(u, v));
        auto ray_direction = ray_end - ray_start;

        auto ray_projection = glm::dot(normal, ray_direction);
        if (std::abs(ray_projection) >= 0.00001f)
        {
          auto intersect_point_1d = glm::dot(normal, triangle[0] - ray_start) / ray_projection;
          if (intersect_point_1d >= 0.0f && intersect_point_1d <= 1.0f)
          {
            auto intersect_point = ray_start + intersect_point_1d * ray_direction;

            auto uu = glm::dot(u, u);
            auto uv = glm::dot(u, v);
            auto vv = glm::dot(v, v);
            auto w = intersect_point - triangle[0];
            auto wu = dot(w, u);
            auto wv = dot(w, v);
            auto d = uv * uv - uu * vv;

            auto s = (uv * wv - vv * wu) / d;
            if (s >= 0.0 && s <= 1.0)
            {
              auto t = (uv * wu - uu * wv) / d;
              if (t >= 0.0 && s + t <= 1.0)
              {
                Vector3f result = { intersect_point.x, intersect_point.y, intersect_point.z };
                return std::make_pair(intersect_point_1d, result);
              }
            }
          }
        }

        return boost::none;
      }
    }

    boost::optional<Vector3f> 
      TerrainScene::find_terrain_position_at(Vector2i absolute_position, Vector2i screen_size,
                                             IntRect view_port, const resources_3d::HeightMap& height_map,
                                             const glm::mat4& projected_view) const
    {
      if (screen_size.x == 0 || screen_size.y == 0) return boost::none;

      // Need to get the relative position *in the viewport*.      
      Vector2f relative_position;
      {
        auto left = view_port.left - (screen_size.x - view_port.width) / 2;
        auto bottom = (screen_size.y - view_port.height) -
          view_port.top - (screen_size.y - view_port.height) / 2;

        absolute_position.y = screen_size.y - absolute_position.y;
        relative_position = vector2_cast<float>((absolute_position - make_vector2(left, bottom))) /
          (vector2_cast<float>(screen_size) * 0.5f) - 1.0f;
      }

      auto inverse_view = glm::inverse(projected_view);

      auto intermediate_z = inverse_view[0].z * relative_position.x +
        inverse_view[1].z * relative_position.y + inverse_view[3].z;
      auto intermediate_w = inverse_view[0].w * relative_position.y +
        inverse_view[1].w * relative_position.y + inverse_view[3].w;

      auto calculate_projection_z_coord = [&](float world_z)
      {
        return (intermediate_z - (intermediate_w * world_z)) / (-inverse_view[2].z + world_z * inverse_view[2].w);
      };

      auto calculate_world_coords_from_projection_z = [&](float projection_z)
      {
        auto vec = inverse_view * glm::vec4(relative_position.x, relative_position.y, projection_z, 1.0f);
        return vec /= vec.w;
      };

      auto track_height = static_cast<float>(track_size_.z);
      auto bounds = std::make_pair(calculate_projection_z_coord(0.0f),
                                   calculate_projection_z_coord(track_height));

      auto ray_start_4d = inverse_view * glm::vec4(relative_position.x, relative_position.y, bounds.first, 1.0f);
      auto ray_end_4d = inverse_view * glm::vec4(relative_position.x, relative_position.y, bounds.second, 1.0f);

      ray_start_4d /= ray_start_4d.w;
      ray_end_4d /= ray_end_4d.w;

      auto ray_start = glm::vec3(ray_start_4d);
      auto ray_end = glm::vec3(ray_end_4d);

      auto world_bounds = make_vector2(static_cast<float>(track_size_.x), static_cast<float>(track_size_.y));

      auto bounding_box = intersection(make_rect_from_points(make_vector2(ray_start.x, ray_start.y),
                                                             make_vector2(ray_end.x, ray_end.y)),
                                       make_rect_from_points(make_vector2(0.0f, 0.0f), world_bounds));

      auto intersection_cmp = [](const auto& a, const auto& b)
      {
        if (!a) return false;
        if (!b) return true;

        return a->first < b->first;
      };

      if (bounding_box.width > 0.0f && bounding_box.height > 0.0f)
      {
        auto cell_size = height_map.cell_size();

        IntRect map_bounds;
        map_bounds.left = static_cast<std::int32_t>(bounding_box.left) / cell_size;
        map_bounds.top = static_cast<std::int32_t>(bounding_box.top) / cell_size;
        map_bounds.width = static_cast<std::int32_t>(bounding_box.right()) / cell_size + 1 - map_bounds.left;
        map_bounds.height = static_cast<std::int32_t>(bounding_box.bottom()) / cell_size + 1 - map_bounds.top;

        boost::optional<std::pair<float, Vector3f>> result;

        // Loop through all height map cells that intersect with the potential area.
        std::size_t idx = 0;
        for (auto y = map_bounds.top, bottom = map_bounds.bottom(); y != bottom; ++y)
        {
          for (auto x = map_bounds.left, right = map_bounds.right(); x != right; ++x)
          {
            // Then, see if the screen position is inside the height map cell.
            auto top_left = make_vector2(static_cast<float>(x),
                                         static_cast<float>(y));
            auto bottom_right = top_left + 1.0f;

            top_left *= static_cast<float>(cell_size);
            bottom_right *= static_cast<float>(cell_size);

            auto cell = make_rect_from_points(top_left, bottom_right);

            std::array<glm::vec3, 3> first_triangle =
            {
              {
                { cell.left, cell.top, height_map(x, y) },
                { cell.right(), cell.top, height_map(x + 1, y) },
                { cell.left, cell.bottom(), height_map(x, y + 1) }
              }
            };

            std::array<glm::vec3, 3> second_triangle =
            {
              {
                { cell.right(), cell.top, height_map(x + 1, y) },
                { cell.left, cell.bottom(), height_map(x, y + 1) },
                { cell.right(), cell.bottom(), height_map(x + 1, y + 1) }
              }
            };

            auto first_intersection = detail::triangle_ray_intersection(first_triangle,
                                                                        ray_start,
                                                                        ray_end);

            auto second_intersection = detail::triangle_ray_intersection(second_triangle,
                                                                         ray_start,
                                                                         ray_end);

            auto cmp = [](const auto& a, const auto& b)
            {
              if (!a) return false;
              if (!b) return true;

              return a->first < b->first;
            };

            const auto& intersection = std::min(first_intersection, second_intersection, cmp);
            if (cmp(intersection, result))
            {
              result = intersection;
            }
          }
        }

        if (result) return result->second;
      }

      else
      {
        // TODO: find out-of-world-bounds positions
        static constexpr float boundary = 1000000.0f;
        const glm::vec3 first_triangle[] =
        {
          { -boundary, -boundary, 0.0f },
          { boundary, -boundary, 0.0f },
          { -boundary, boundary, 0.0f }
        };

        const glm::vec3 second_triangle[] =
        {
          { boundary, -boundary, 0.0f },
          { -boundary, boundary, 0.0f },
          { boundary, boundary, 0.0f }
        };

        auto first_intersection = detail::triangle_ray_intersection(first_triangle,
                                                                    ray_start,
                                                                    ray_end);

        auto second_intersection = detail::triangle_ray_intersection(second_triangle,
                                                                     ray_start,
                                                                     ray_end);

        const auto& intersection = std::min(first_intersection, second_intersection, intersection_cmp);
        if (intersection)
        {
          return intersection->second;
        }
      }

      return boost::none;
    }
  }
}