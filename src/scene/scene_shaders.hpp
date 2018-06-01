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
        in vec2 in_position;
        in vec2 in_texCoords;
        out vec2 frag_texCoords;
        void main()
        {
          frag_texCoords = in_texCoords;
          gl_Position = u_viewMatrix * vec4(in_position, 0.0, 1.0);
        }
      )";

      static const char track_fragment_shader[] = R"(
        #version 130
        uniform sampler2D u_textureSampler;
        in vec2 frag_texCoords;
        out vec4 frag_color;
        void main()
        {
          frag_color = texture2D(u_textureSampler, frag_texCoords);
        };
      )";

      static const char car_vertex_shader[] = R"(
        #version 130
        uniform mat4 u_viewMatrix;
        uniform mat4 u_modelMatrix;
        uniform mat4 u_newModelMatrix;
        uniform mat4 u_colorizerMatrix;
        uniform vec4 u_carColors[3];
        uniform float u_frameProgress;
        in vec2 in_position;
        in vec2 in_texCoord;
        in vec3 in_colorizerCoord;
        out vec2 frag_texCoord;
        out vec3 frag_colorizerCoord;
        out vec3 frag_carColors[3];
        void main()
        {
          frag_texCoord = in_texCoord;
          vec4 transformedCoord = u_colorizerMatrix * vec4(in_colorizerCoord.xy, 0.0, 1.0);
          frag_colorizerCoord = vec3(transformedCoord.xy, in_colorizerCoord.z);
          vec4 position = u_viewMatrix * u_modelMatrix * vec4(in_position, 0.0, 1.0);
          vec4 newPosition = u_viewMatrix * u_newModelMatrix * vec4(in_position, 0.0, 1.0);
          gl_Position = mix(position, newPosition, u_frameProgress);
        }
      )";

      static const char car_fragment_shader[] = R"(
        #version 130
        uniform sampler2D u_textureSampler;
        uniform sampler2D u_colorizerSampler;
        in vec2 frag_texCoord;
        in vec3 frag_colorizerCoord;
        in vec3 frag_carColors[3];
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
          vec4 textureColor = texture2D(u_textureSampler, frag_texCoord);
          vec4 colorizerColor = texture2D(u_colorizerSampler, frag_colorizerCoord.xy);
          vec4 totalColor = colorizerColor.r * vec4(frag_carColors[0], 1.0) + 
                            colorizerColor.g * vec4(frag_carColors[1], 1.0) + 
                            colorizerColor.b * vec4(frag_carColors[2], 1.0);
          vec4 avgColor = totalColor / 3.0; avgColor = vec4(1.0, 0.85, 0.0, 1.0);
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
        in vec2 in_texCoord;
        out vec2 frag_texCoord;        
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
        in vec2 frag_texCoord;
        out vec4 out_fragColor;
        void main()
        {
          vec4 texColor = texture(u_texSampler, frag_texCoord);
          out_fragColor = vec4(u_shadowColor.rgb, u_shadowColor.a * texColor.a);
        }
      )";

      static const char particle_vertex_shader[] = R"(
        #version 130
        uniform mat4 u_viewMatrix;
        in vec2 in_position;
        in vec2 in_center;
        in vec4 in_color;
        in float in_radius;
        out vec2 frag_position;
        out vec2 frag_center;
        out vec4 frag_color;        
        out float frag_radius;
        void main()
        {
          gl_Position = u_viewMatrix * vec4(in_position.xy, 0.0, 1.0);
          frag_position = in_position.xy;
          frag_center = in_center.xy;
          frag_color = in_color;
          frag_radius = in_radius;          
        }
      )";

      static const char particle_fragment_shader[] = R"(
        #version 130
        in vec2 frag_position;
        in vec2 frag_center;
        in vec4 frag_color;
        in float frag_radius;
        out vec4 out_fragColor;
        void main()
        {
          float dist = distance(frag_position, frag_center);
          float alpha = smoothstep(frag_radius - 0.05, frag_radius, dist);
          out_fragColor = frag_color;
          out_fragColor.a = 1.0 - alpha;
        }
      )";

      static const char boundary_vertex_shader[] = R"(
      #version 130
      uniform mat4 u_viewMatrix;
      uniform vec2 u_worldSize;
      in vec2 in_position;
      void main()
      {
        gl_Position = u_viewMatrix * vec4(in_position * u_worldSize, 0, 1);
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
    }
  }
}
