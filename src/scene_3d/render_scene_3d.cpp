/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "render_scene_3d.hpp"
#include "viewport_3d.hpp"

#include "resources_3d/track_3d.hpp"
#include "resources_3d/elevation_map_3d.hpp"
#include "resources_3d/terrain_model_3d.hpp"
#include "resources_3d/model_3d.hpp"
#include "resources_3d/path_vertices_3d.hpp"

#include "graphics/gl_check.hpp"
#include "graphics/gl_scissor_box.hpp"
#include "graphics/image_loader.hpp"

#include "utility/math_utilities.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <numeric>

namespace ts
{
  namespace scene3d
  {
    RenderScene::RenderScene(const resources3d::Track& track)
      : terrain_shader_program_(graphics::create_shader_program(shaders::terrain_vertex_shader, shaders::terrain_fragment_shader)),       
        terrain_vertex_buffer_(graphics::create_buffer()),
        terrain_element_buffer_(graphics::create_buffer()),
        terrain_vertex_array_(graphics::create_vertex_array()),
        terrain_uniforms_(shaders::terrain_shader_uniform_locations(terrain_shader_program_))
    {
      load_textures(track.texture_library());
      load_terrain_geometry(track);
    }

    void RenderScene::load_textures(const resources3d::TextureLibrary& tex_library)
    {
      auto textures = tex_library.textures();
      using resources3d::TextureDescriptor;      

      // Sort the textures by size. We are guaranteed that the dimensions of their image rects are powers of two.
      std::sort(textures.begin(), textures.end(), 
                [](const TextureDescriptor& a, const TextureDescriptor& b)
      {
        return std::tie(a.image_rect.width, a.image_rect.height) < 
          std::tie(b.image_rect.width, b.image_rect.height);
      });

      graphics::DefaultImageLoader image_loader;

      // Now, load the textures' images
      for (auto it = textures.begin(); it != textures.end(); )
      {
        const auto image_rect = it->image_rect;
        auto texture_size = make_vector2(image_rect.width, image_rect.height);

        auto range_end = std::find_if(std::next(it), textures.end(),
                                      [=](const TextureDescriptor& tex_desc)
        {
          return tex_desc.image_rect.width != image_rect.width;
        });

        auto depth = static_cast<std::uint32_t>(std::distance(it, range_end));

        GLuint tex{};
        glGenTextures(1, &tex);
        generic_textures_.emplace_back(tex, Vector3u(texture_size.x, texture_size.y, depth));

        std::uint32_t levels = 0;
        for (auto s = texture_size; s.x != 0 && s.y != 0; s.x >>= 1, s.y >>= 1, ++levels) {}

        glBindTexture(GL_TEXTURE_2D_ARRAY, tex);       
        glCheck(glTexStorage3D(GL_TEXTURE_2D_ARRAY, levels, GL_RGBA8, texture_size.x, texture_size.y, depth));

        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        if (GLEW_EXT_texture_filter_anisotropic)
        {
          glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
        }       

        // Loop through all the entries that have the same size as the one referenced by 'it'.
        for (GLint index = 0; it != range_end; ++it, ++index)
        {
          const auto& image = image_loader.load_image(it->image_file);
          const auto image_size = image.getSize();

          auto bounds = rect_cast<std::uint32_t>(it->image_rect);

          if (bounds.left >= image_size.x || bounds.right() > image_size.x ||
              bounds.top >= image_size.y || bounds.bottom() > image_size.y)
          {
            auto area_string = std::to_string(it->image_rect.left) + " " + std::to_string(it->image_rect.top) + " " +
              std::to_string(it->image_rect.width) + " " + std::to_string(it->image_rect.height);

            throw std::runtime_error("texture area out of bounds (" + it->image_file + "; " + area_string + ")");
          }          

          glPixelStorei(GL_UNPACK_ROW_LENGTH, image.getSize().x);
          glPixelStorei(GL_UNPACK_SKIP_PIXELS, it->image_rect.left);
          glPixelStorei(GL_UNPACK_SKIP_ROWS, it->image_rect.top);
          
          glCheck(glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, index, it->image_rect.width, it->image_rect.height, 1,
                                  GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr()));

          // Now, store the texture in our lookup map.
          if (it->internal_id >= texture_lookup_.size())
          {
            texture_lookup_.resize(it->internal_id + 1);
          }
          
          auto& entry = texture_lookup_[it->internal_id];
          entry.level = index;
          entry.texture = &generic_textures_.back();
        }

        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
      }

      glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
      glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
      glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
      glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    }

    namespace detail
    {
      auto generate_base_terrain_model(Vector2f min_corner, Vector2f max_corner, float max_region_size,
                                       Vector2f texture_scale, float texture_z, Colorb color)
      {
        resources3d::Model model;
        auto make_vertex = [=](Vector2f point)
        {
          resources3d::Vertex vertex;
          vertex.position = { make_3d(point, 0.0f) };
          vertex.normal = { 0.0f, 0.0f, 1.0f };
          vertex.tex_coords = make_3d(point * texture_scale, texture_z);
          vertex.color = color;
          return vertex;
        };

        auto total_size = max_corner - min_corner;

        auto horizontal_regions = static_cast<std::int32_t>(std::ceil(total_size.x / max_region_size));
        auto vertical_regions = static_cast<std::int32_t>(std::ceil(total_size.y / max_region_size));

        auto region_size = total_size / make_vector2(horizontal_regions, vertical_regions);

        auto position = min_corner;
        for (std::int32_t y = 0; y < vertical_regions; ++y, position.y += region_size.y)
        {
          position.x = min_corner.x;
          for (std::int32_t x = 0; x < horizontal_regions; ++x, position.x += region_size.x)
          {
            auto tl_vertex_index = static_cast<std::uint32_t>(model.vertices.size());
            auto tr_vertex_index = tl_vertex_index + 1;
            auto bl_vertex_index = tl_vertex_index + horizontal_regions + 1;
            auto br_vertex_index = bl_vertex_index + 1;

            model.vertices.push_back(make_vertex(position));

            model.faces.insert(model.faces.end(),
            {
              { tl_vertex_index, bl_vertex_index, br_vertex_index },
              { br_vertex_index, tr_vertex_index, tl_vertex_index }
            });
          }

          model.vertices.push_back(make_vertex(position));
        }

        // Add last row
        position.x = min_corner.x;
        for (std::int32_t x = 0; x <= horizontal_regions; ++x, position.x += region_size.x)
        {
          model.vertices.push_back(make_vertex(position));
        }

        return model;
      }
    }

    void RenderScene::load_terrain_geometry(const resources3d::Track& track)
    {
      const auto& elevation_map = track.elevation_map();
      const auto world_size = track.size();
      const auto additional_terrain_size = Vector2f(5000.0f, 5000.0f);

      terrain_texture_ = &generic_textures_.front();
      auto texture_scale = 4.0f / make_2d(terrain_texture_->size());

      auto base_terrain = detail::generate_base_terrain_model(-additional_terrain_size, 
                                                              world_size + additional_terrain_size, 512.0f,
                                                              texture_scale, 0.0f, Colorb(255, 255, 255, 255));

      resources3d::TerrainBuilder terrain_builder;
      terrain_builder.apply_model(base_terrain, 0);

      std::vector<resources3d::PathVertexPoint> path_vertex_points;

      for (const auto& path_layer : track.path_layers())
      {
        for (const auto& path : path_layer->paths)
        {
          resources3d::compute_path_vertex_points(path.nodes.begin(), path.nodes.end(), 0.05f, path_vertex_points);

          for (const auto& stroke_style : path.stroke_styles)
          {
            auto path_model = resources3d::build_path_model(stroke_style, path_vertex_points, 
                                                            elevation_map, 0.0f, texture_scale, path.closed);

            //terrain_builder.apply_model(path_model, 0);
          }
        }
      }

      auto terrain_model = terrain_builder.build_model(elevation_map);

      using Vertex = resources3d::Vertex;
      using Face = resources3d::Face;
      
      terrain_components_.clear();
      for (const auto& c : terrain_model.components)
      {
        TerrainComponent component;
        component.element_index = reinterpret_cast<const void*>(static_cast<std::uintptr_t>(c.face_index) * sizeof(Face));
        component.element_count = c.face_count * 3;
        component.texture = texture_lookup_[c.model_tag].texture;
        terrain_components_.push_back(component);

        auto v = terrain_model.vertices.front();
      }

      auto vertex_data_size = terrain_model.vertices.size() * sizeof(Vertex);
      auto element_data_size = terrain_model.faces.size() * sizeof(Face);

      auto vertex_buffer_size = utility::next_power_of_two(vertex_data_size);
      auto element_buffer_size = utility::next_power_of_two(element_data_size);

      auto vao = terrain_vertex_array_.get();
      glBindVertexArray(terrain_vertex_array_.get());

      glBindBuffer(GL_ARRAY_BUFFER, terrain_vertex_buffer_.get());
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrain_element_buffer_.get());
      
      glCheck(glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size, nullptr, GL_STATIC_DRAW));
      glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER, element_buffer_size, nullptr, GL_STATIC_DRAW));

      glCheck(glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_data_size, terrain_model.vertices.data()));
      glCheck(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, element_data_size, terrain_model.faces.data()));

      glCheck(glEnableVertexAttribArray(0));
      glCheck(glEnableVertexAttribArray(1));
      glCheck(glEnableVertexAttribArray(2));
      glCheck(glEnableVertexAttribArray(3));

      glCheck(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                    reinterpret_cast<const void*>(offsetof(Vertex, position))));

      glCheck(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                    reinterpret_cast<const void*>(offsetof(Vertex, tex_coords))));

      glCheck(glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                    reinterpret_cast<const void*>(offsetof(Vertex, normal))));

      glCheck(glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex),
                                    reinterpret_cast<const void*>(offsetof(Vertex, color))));

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, 1);
      glBindVertexArray(0);
    }

    void RenderScene::render(const Viewport& view_port, Vector2i screen_size, double frame_progress) const
    {
      glCheck(glEnable(GL_TEXTURE_2D));
      glCheck(glDisable(GL_CULL_FACE));
      glCheck(glDisable(GL_DEPTH_TEST));
      glCheck(glEnable(GL_BLEND));
      glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

      auto screen_rect = view_port.screen_rect();

      glCheck(glViewport(screen_rect.left, screen_size.y - screen_rect.bottom(),
                         screen_rect.width, screen_rect.height));

      graphics::scissor_box(view_port.screen_rect(), screen_size);      

      auto mat = projection_matrix(view_port);

      glUseProgram(terrain_shader_program_.get());

      glUniformMatrix4fv(terrain_uniforms_.view_matrix, 1, GL_FALSE, glm::value_ptr(mat));
      glUniform3f(terrain_uniforms_.light_direction, 0.0f, 0.0f, 1.0f);
      glUniform1i(terrain_uniforms_.texture_sampler, 0);

      glBindVertexArray(terrain_vertex_array_.get());
      glBindBuffer(GL_ARRAY_BUFFER, terrain_vertex_buffer_.get());
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrain_element_buffer_.get());

      using Vertex = resources3d::Vertex;
      
      glActiveTexture(GL_TEXTURE0);
      for (const auto& component : terrain_components_)
      {
        glBindTexture(GL_TEXTURE_2D_ARRAY, component.texture->get());

        glCheck(glDrawElements(GL_TRIANGLES, component.element_count, GL_UNSIGNED_INT, component.element_index));
      }     

      graphics::disable_scissor_box();

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
      glUseProgram(0);
    }

    void RenderScene::update(std::int32_t frame_duration)
    {

    }
  }
}