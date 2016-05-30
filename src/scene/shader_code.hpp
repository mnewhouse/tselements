/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef SHADER_CODE_HPP_6828976
#define SHADER_CODE_HPP_6828976

namespace ts
{
  namespace scene
  {
    namespace shaders
    {
      static const char track_vertex_shader[] = R"(
        #version 330
        uniform mat4 viewMat;
        layout(location = 0) in vec2 in_position;
        layout(location = 1) in vec2 in_texCoord;
        out vec2 frag_worldPosition;
        out vec2 frag_texCoord;
        void main()
        {
          gl_Position = viewMat * vec4(in_position.xy, 0.0, 1.0);
          frag_texCoord = in_texCoord;
          frag_worldPosition = in_position;
        }
      )";

      static const char track_fragment_shader[] = R"(
        #version 330
        in vec2 frag_worldPosition;
        in vec2 frag_texCoord;
        out vec4 out_fragColor;
        uniform sampler2D texSampler;
        uniform vec2 worldSize;
        void main()
        {
          if (frag_worldPosition.x < 0.0 || frag_worldPosition.y < 0.0 ||
              frag_worldPosition.x >= worldSize.x|| frag_worldPosition.y >= worldSize.y)
          {
            discard;
          }
          else
          {
            out_fragColor = texture(texSampler, frag_texCoord);
          }
        };
      )";

      static const char car_vertex_shader[] = R"(
        #version 330
        uniform mat4 viewMat;
        layout(location = 0) in vec2 in_position;        
        layout(location = 1) in vec2 in_texCoord;
        layout(location = 2) in vec2 in_frameOffset;
        layout(location = 3) in vec2 in_colorizerCoord;
        layout(location = 4) in vec4 in_colorizerBounds;
        layout(location = 5) in vec4 in_primaryColor;
        layout(location = 6) in vec4 in_secondaryColor;
        layout(location = 7) in vec4 in_tertiaryColor;
        uniform float frameProgress;
        out vec2 frag_worldPosition;
        out vec2 frag_texCoord;
        out vec2 frag_colorizerCoord;
        out vec4 frag_colorizerBounds;
        out vec4 frag_primaryColor;
        out vec4 frag_secondaryColor;
        out vec4 frag_tertiaryColor;
        void main()
        {
          vec2 position = in_position + in_frameOffset * frameProgress;
          gl_Position = viewMat * vec4(position, 0.0, 1.0);
          frag_texCoord = in_texCoord;
          frag_worldPosition = position;
          frag_colorizerCoord = in_colorizerCoord;
          frag_colorizerBounds = in_colorizerBounds;
          frag_primaryColor = in_primaryColor;
          frag_secondaryColor = in_secondaryColor;
          frag_tertiaryColor = in_tertiaryColor;          
        }
      )";

      static const char car_fragment_shader[] = R"(
        #version 330
        in vec2 frag_worldPosition;
        in vec2 frag_texCoord;
        in vec2 frag_colorizerCoord;
        in vec4 frag_colorizerBounds;
        in vec4 frag_primaryColor;
        in vec4 frag_secondaryColor;
        in vec4 frag_tertiaryColor;
        out vec4 out_fragColor;
        uniform sampler2D texSampler;
        uniform sampler2D colorSampler;
        uniform vec2 worldSize;
        vec4 colorize(vec4 source, vec4 target)
        {
          float minValue = min(source.r, min(source.g, source.b));
          float maxValue = max(source.r, max(source.g, source.b));
          float redAmount = max(source.r - abs(source.g - source.b), 0) / maxValue;
          vec4 result = vec4
          (
            (source.rgb + (target.rgb - source.rgb) * redAmount) * maxValue,
            source.a
          );
          result.r = clamp(result.r, minValue, maxValue);
          result.g = clamp(result.g, minValue, maxValue);
          result.b = clamp(result.b, minValue, maxValue);          
          return result;         
        }
        void main()
        {
          if (frag_worldPosition.x < 0.0 || frag_worldPosition.y < 0.0 ||
              frag_worldPosition.x >= worldSize.x || frag_worldPosition.y >= worldSize.y)
          {
            discard;
          }
          else
          {
            vec4 textureColor = texture(texSampler, frag_texCoord);
            vec2 colorizerCoord = clamp(frag_colorizerCoord, frag_colorizerBounds.xy, 
                                        frag_colorizerBounds.xy + frag_colorizerBounds.zw);
            vec4 colorizerColor = texture(colorSampler, frag_colorizerCoord);
            vec4 totalColor = colorizerColor.r * frag_primaryColor + 
                              colorizerColor.g * frag_secondaryColor + 
                              colorizerColor.b * frag_tertiaryColor;
            vec4 avgColor = totalColor / (colorizerColor.r + colorizerColor.g + colorizerColor.b);
            out_fragColor = colorize(textureColor, avgColor);
          }
        };
      )";

      static const char shadow_vertex_shader[] = R"(
        #version 330
        uniform mat4 viewMat;
        uniform float frameProgress;
        layout(location = 0) in vec2 in_position;
        layout(location = 1) in vec2 in_texCoord;
        layout(location = 2) in vec2 in_frameOffset;
        layout(location = 8) in float in_hoverDistance;
        out vec2 frag_worldPosition;
        out vec2 frag_texCoord;        
        void main()
        {
          vec2 distance = vec2(in_hoverDistance, in_hoverDistance) + 1.5;
          vec2 position = in_position.xy + distance + in_frameOffset * frameProgress;
          frag_worldPosition = position;
        
          gl_Position = viewMat * vec4(position, 0.0, 1.0);
          frag_texCoord = in_texCoord;
        }    
      )";

      static const char shadow_fragment_shader[] = R"(
        #version 330
        uniform vec4 shadowColor;
        uniform vec2 worldSize;
        uniform sampler2D texSampler;        

        in vec2 frag_worldPosition;
        in vec2 frag_texCoord;
        out vec4 out_fragColor;

        void main()
        {
          if (frag_worldPosition.x < 0.0 || frag_worldPosition.y < 0.0 ||
              frag_worldPosition.x >= worldSize.x|| frag_worldPosition.y >= worldSize.y)
          {
            discard;
          }

          vec4 texColor = texture(texSampler, frag_texCoord);
          out_fragColor = vec4(shadowColor.rgb, shadowColor.a * texColor.a);
        }
      )";

      static const char particle_vertex_shader[] = R"(
        #version 330
        uniform mat4 viewMat;
        layout(location = 0) in vec2 in_position;
        layout(location = 1) in vec2 in_center;
        layout(location = 2) in vec4 in_color;
        layout(location = 3) in float in_radius;
        out vec2 frag_position;
        out vec2 frag_center;
        out vec4 frag_color;        
        out float frag_radius;
        void main()
        {
          gl_Position = viewMat * vec4(in_position.xy, 0.0, 1.0);
          frag_position = in_position.xy;
          frag_center = in_center.xy;
          frag_color = in_color;
          frag_radius = in_radius;          
        }
      )";

      static const char particle_fragment_shader[] = R"(
        #version 330
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

    }
  }
}

#endif