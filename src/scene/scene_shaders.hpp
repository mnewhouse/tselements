/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

namespace ts
{
  namespace scene
  {
    namespace shaders
    {
      static const char track_vertex_shader[] = R"(
        #version 130
        uniform mat4 u_viewMatrix;        
        in vec3 in_position;
        in vec2 in_texCoords;   
        out vec2 frag_position;     
        out vec2 frag_texCoords;
        void main()
        {          
          frag_position = in_position.xy;
          frag_texCoords = in_texCoords;
          gl_Position = u_viewMatrix * vec4(frag_position, 0.0, 1.0);
        }
      )";

      static const char track_fragment_shader[] = R"(
        #version 130
        uniform vec2 u_minCorner;
        uniform vec2 u_maxCorner;
        uniform sampler2D u_textureSampler;
        in vec2 frag_position;
        in vec2 frag_texCoords;        
        out vec4 frag_color;
        void main()
        {
          if (frag_position.x < u_minCorner.x || frag_position.y < u_minCorner.y ||
              frag_position.x > u_maxCorner.x || frag_position.y > u_maxCorner.y)
          {
            discard;
          }

          else
          {
            frag_color = texture2D(u_textureSampler, frag_texCoords);
          }
        };
      )";

      static const char track_path_vertex_shader[] = R"(
        #version 130
        uniform mat4 u_viewMatrix;
        uniform float u_zBase;
        uniform float u_zScale;   
        in vec3 in_position;
        in vec2 in_texCoords;   
        out vec2 frag_position;     
        out vec2 frag_texCoords;
        void main()
        {
          float z = u_zBase + in_position.z * u_zScale;
          frag_position = in_position.xy;
          frag_texCoords = in_texCoords;
          gl_Position = u_viewMatrix * vec4(frag_position.xy, z, 1.0);
        }
      )";

      static const char track_path_fragment_shader[] = R"(
        #version 130
        uniform sampler2D u_weightSampler;
        uniform sampler2D u_primarySampler;
        uniform sampler2D u_secondarySampler;
        uniform vec2 u_primaryScale;
        uniform vec2 u_secondaryScale;
        uniform vec2 u_minCorner;
        uniform vec2 u_maxCorner;
        
        in vec2 frag_position;
        in vec2 frag_texCoords;        
        out vec4 frag_color;
        void main()
        {
          if (frag_position.x < u_minCorner.x || frag_position.y < u_minCorner.y ||
              frag_position.x > u_maxCorner.x || frag_position.y > u_maxCorner.y)
          {
            discard;
          }

          else
          {
            vec4 weights = texture2D(u_weightSampler, frag_texCoords);
            vec4 primary = texture2D(u_primarySampler, frag_position * u_primaryScale) * weights.r;
            vec4 secondary = texture2D(u_secondarySampler, frag_position * u_secondaryScale) * weights.g;                       

            frag_color = (primary + secondary) / max(weights.r + weights.g, 1.0);
          }
        };
      )";

      static const char car_vertex_shader[] = R"(
        #version 130
        uniform mat4 u_viewMatrix;
        uniform mat4 u_modelMatrix;
        uniform mat4 u_newModelMatrix;
        uniform mat4 u_colorizerMatrix;
        uniform vec2 u_texCoordsOffset;
        uniform vec2 u_texCoordsScale;
        uniform float u_frameProgress;

        in vec2 in_position;
        in vec2 in_texCoords;
        out vec2 frag_texCoords;
        out vec2 frag_colorizerCoords;
        void main()
        {
          frag_texCoords = in_texCoords * u_texCoordsScale + u_texCoordsOffset;
          frag_colorizerCoords = (u_colorizerMatrix * vec4(in_texCoords.xy, 0.0, 1.0)).xy;
   
          vec4 position = u_viewMatrix * u_modelMatrix * vec4(in_position, 0.0, 1.0);
          vec4 newPosition = u_viewMatrix * u_newModelMatrix * vec4(in_position, 0.0, 1.0);
          gl_Position = mix(position, newPosition, u_frameProgress);
        }
      )";

      static const char car_fragment_shader[] = R"(
        #version 130
        uniform sampler2D u_textureSampler;
        uniform sampler2D u_colorizerSampler;
        uniform vec3 u_carColors[3];
        in vec2 frag_texCoords;
        in vec2 frag_colorizerCoords;
        out vec4 frag_color;
        vec4 colorize(vec4 source, vec4 target)
        {
          float minValue = min(source.r, min(source.g, source.b));
          float maxValue = max(source.r, max(source.g, source.b));
          float redAmount = max(source.r - abs(source.g - source.b), 0) / maxValue;
          vec4 result = vec4
          (
            (source.rgb + (target.rgb - source.rgb) * redAmount) * maxValue, source.a
          );
          result.r = clamp(result.r, minValue, maxValue);
          result.g = clamp(result.g, minValue, maxValue);
          result.b = clamp(result.b, minValue, maxValue);          
          return result;         
        }
        void main()
        {
          vec4 textureColor = texture2D(u_textureSampler, frag_texCoords);
          vec4 colorizerColor = texture2D(u_colorizerSampler, frag_colorizerCoords);
          vec3 totalColor = colorizerColor.r * u_carColors[0] + 
                            colorizerColor.g * u_carColors[1] + 
                            colorizerColor.b * u_carColors[2];
          
          float d = colorizerColor.r + colorizerColor.g + colorizerColor.b;
          vec4 avgColor = vec4(totalColor / max(d, 1.0), 1.0);
          frag_color = colorize(textureColor, avgColor);
        };
      )";

      static const char shadow_vertex_shader[] = R"(
        #version 130
        uniform mat4 u_viewMatrix;
        uniform mat4 u_modelMatrix;
        uniform mat4 u_newModelMatrix;
        uniform float u_frameProgress;
        uniform vec2 u_shadowOffset;
        in vec2 in_position;
        in vec2 in_texCoords;
        out vec2 frag_texCoords;        
        void main()
        {
          vec4 position = u_modelMatrix * in_position;
          vec4 newPosition = u_newModelMatrix * in_position;
          vec4 interpolatedPosition = mix(position, newPosition, u_frameProgress) + u_shadowOffset;          
          frag_texCoord = in_texCoord;
          gl_Position = u_viewMatrix * interpolatedPosition;
        }    
      )";

      static const char shadow_fragment_shader[] = R"(
        #version 130
        uniform vec4 u_shadowColor;
        uniform sampler2D u_texSampler;
        in vec2 frag_texCoords;
        out vec4 out_fragColor;
        void main()
        {
          vec4 texColor = texture(u_texSampler, frag_texCoords);
          out_fragColor = vec4(u_shadowColor.rgb, u_shadowColor.a * texColor.a);
        }
      )";

      static const char boundary_vertex_shader[] = R"(
      #version 130
      uniform mat4 u_viewMatrix;
      uniform vec2 u_worldSize;
      in vec2 in_position;
      void main()
      {
        gl_Position = u_viewMatrix * vec4(in_position * u_worldSize, 1, 1);
      }
    )";

      static const char boundary_fragment_shader[] = R"(
      #version 130
      out vec4 out_fragColor;
      void main()
      {
        out_fragColor = vec4(0, 0, 0, 1);
      }
      )";

      static const char particle_vertex_shader[] = R"(
        #version 130
        uniform mat4 u_viewMatrix;
        in vec2 in_position;
        in vec2 in_texCoords;
        in vec4 in_color;
        out vec2 frag_texCoords;
        out vec4 frag_color;
        void main()
        {
          frag_texCoords = in_texCoords;
          frag_color = in_color;
          gl_Position = u_viewMatrix * vec4(in_position, 0, 1);          
        }
      )";

      static const char particle_fragment_shader[] = R"(
        #version 130
        uniform sampler2D u_textureSampler;
        in vec2 frag_texCoords;
        in vec4 frag_color;
        out vec4 out_fragColor;
        void main()
        {          
          out_fragColor = frag_color * texture(u_textureSampler, frag_texCoords);
        }
      )";
    }
  }
}
