/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "shaders/scene_3d_shaders.hpp"

#include "graphics/buffer.hpp"
#include "graphics/texture.hpp"
#include "graphics/shader.hpp"

#include "utility/vector2.hpp"

#include <glm/mat4x4.hpp>

#include <vector>
#include <list>
#include <cstdint>

namespace ts
{
  namespace resources3d
  {
    class Track;
    class ElevationMap;
    class TextureLibrary;
  }

  namespace scene3d
  {
    class Viewport;

    class RenderScene
    {
    public:
      explicit RenderScene(const resources3d::Track& track);

      void render(const Viewport& viewport, Vector2i screen_size, double frame_progress) const;
      void update(std::int32_t frame_duration);

      void load_terrain_geometry(const resources3d::Track& track);

    private:      
      void load_textures(const resources3d::TextureLibrary& texture_library);

      graphics::ShaderProgram terrain_shader_program_;
      graphics::ShaderProgram model_shader_program_;

      graphics::Buffer terrain_vertex_buffer_;
      graphics::Buffer terrain_element_buffer_;

      graphics::VertexArray terrain_vertex_array_;
      shaders::TerrainShaderUniforms terrain_uniforms_ = {};

      struct TextureLookupEntry
      {
        const graphics::TextureArray* texture;
        std::uint32_t level;
      };
            
      std::list<graphics::TextureArray> generic_textures_;
      std::vector<TextureLookupEntry> texture_lookup_;
      const graphics::TextureArray* terrain_texture_ = nullptr;

      struct TerrainComponent
      {
        const void* element_index;
        std::uint32_t element_count;
        const graphics::TextureArray* texture;
      };      
      
      std::vector<TerrainComponent> terrain_components_;
    };
  }
}