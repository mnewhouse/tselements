/**
** Top down racing game, codenamed TS3D
** Copyright © 2017 Martijn Nijhuis (mniehoes@gmail.com)
**
** This is proprietary software: modifying or distributing this source file
** in any shape or form is not allowed without explicit permission from the author.
**/

#include "path_geometry.hpp"

#include "utility/math_utilities.hpp"

#include <boost/container/small_vector.hpp>

#include <utility>
#include <algorithm>

namespace ts
{
  namespace scene
  {
    struct OutlineIndices
    {
      std::uint32_t first_start, second_start, center_start, end;
      bool closed = false;
    };

    struct OutlinePoint
    {
      float time_point;
      Vector2f point;
      Vector2f normal;
      float width;
    };

    struct OutlineProperties
    {
      bool use_relative_width = true;
      bool invert_normal = false;
      bool center_line = false;
      float width = 1.0f;
      float tolerance = 1.0f;      
    };

    struct BorderProperties
    {
      float width = 0.0f;
      float offset = 0.0f;      
    };

    auto invert(OutlineProperties p)
    {
      p.invert_normal = !p.invert_normal;
      return p;
    }

    namespace detail
    {
      OutlinePoint outline_point_at(const resources::TrackPathNode& a, const resources::TrackPathNode& b, 
                                    float time_point, const OutlineProperties& props)
      {
        auto width = props.use_relative_width ?
          resources::path_width_at(a, b, time_point) * props.width :
          resources::path_width_at(a, b, time_point) + props.width;

        auto normal = resources::path_normal_at(a, b, time_point);
        if (props.invert_normal) normal = -normal;

        auto point = resources::path_point_at(a, b, time_point);
        if (!props.center_line)
        {
          point += normal * std::max(width, 0.0f);
        }

        return
        {
          time_point, point, normal, width
        };
      }

      auto compute_path_point_error(Vector2f start, Vector2f end, 
                                    Vector2f p, float tolerance)
      {
        auto a = end - start;
        auto b = p - start;

        auto cross_error = cross_product(a, b);
        auto dot_error = dot_product(a, b);

        auto error = std::max(cross_error * cross_error, dot_error * dot_error); 
        error = cross_error * cross_error;

        // If p is further from start than end is, use that as our error value.
        return std::make_pair(error, tolerance * tolerance * magnitude_squared(a));
      } 

      void split_outline_segment(const resources::TrackPathNode& a, const resources::TrackPathNode& b,
                                 const OutlinePoint& start, const OutlinePoint& end,
                                 const OutlineProperties& properties, double base_time_point,
                                 std::vector<OutlinePoint>& outline_points)
      {
        auto time_span = end.time_point - start.time_point;
        auto mid = outline_point_at(a, b, start.time_point + time_span * 0.5f, properties);

        auto error = compute_path_point_error(start.point, end.point, mid.point, properties.tolerance);
        if (error.first > error.second)
        {
          // If the error is not within the tolerance level, split this thing in two.
          split_outline_segment(a, b, start, mid, properties, base_time_point, outline_points);
          split_outline_segment(a, b, mid, end, properties, base_time_point, outline_points);
        }

        else
        {
          auto check_direction = [&]()
          {
            auto prev_time = start.time_point - time_span;
            if (prev_time < 0.0) return true;

            auto prev = outline_point_at(a, b, prev_time, properties);
            auto dir = path_direction_at(a, b, start.time_point);
            auto d0 = start.point - prev.point;
            auto d1 = mid.point - start.point;
            
            return dot_product(dir, d0) > 0.0f || dot_product(dir, d1) > 0.0f;
          };

          //if (check_direction())
          {
            // We're within acceptable parameters, add the point to the array and don't recurse any further.   

            outline_points.push_back(start);
            outline_points.back().time_point += base_time_point;
          }

          //else
          {
            // We have a malformed path, we need to find and add a point that's just before
            // where it goes wrong. TODO
          }
        }
      }
    }

    void build_path_segment_outline(const resources::TrackPathNode& a, const resources::TrackPathNode& b, 
                                    std::uint32_t segment_id, const OutlineProperties& properties, 
                                    std::vector<OutlinePoint>& outline_points)
    {
      auto start = detail::outline_point_at(a, b, 0.0f, properties);
      auto mid = detail::outline_point_at(a, b, 0.5f, properties);
      auto end = detail::outline_point_at(a, b, 1.0f, properties);

      auto base_time_point = static_cast<double>(segment_id);

      detail::split_outline_segment(a, b, start, mid, properties, base_time_point, outline_points);
      detail::split_outline_segment(a, b, mid, end, properties, base_time_point, outline_points);
    }    

    void generate_path_outline(const resources::TrackPath& path, const OutlineProperties& outline_properties,
                               std::vector<OutlinePoint>& outline_points)
    {
      // Loop through the path segments, and for each node, compute the points for the outline at the given tolerance.
      if (path.nodes.size() >= 2)
      {
        std::uint32_t segment_id = 0;

        auto node_it = path.nodes.begin(), next_it = std::next(node_it);
        for (; next_it != path.nodes.end(); ++node_it, ++next_it, ++segment_id)
        {          
          build_path_segment_outline(*node_it, *next_it, segment_id, outline_properties, outline_points);
        }

        if (path.closed)
        {
          next_it = path.nodes.begin();

          build_path_segment_outline(*node_it, *next_it, segment_id, outline_properties, outline_points);    
          ++segment_id;
        }

        else
        {
          --next_it;
          --node_it;
        }
                
        outline_points.push_back(detail::outline_point_at(*node_it, *next_it, 1.0f, outline_properties));
        outline_points.back().time_point = static_cast<double>(segment_id); 
      }
    }
    
    void create_base_geometry(const std::vector<OutlinePoint>& points,
                              OutlineIndices outline,
                              std::vector<PathFace>& faces)
    {
      auto make_geometry_between_outlines = [&](std::uint32_t idx_a, std::uint32_t end_a,
                                                std::uint32_t idx_b, std::uint32_t end_b)
      {
        while (true)
        {
          bool a_test = idx_a + 1 == end_a;
          bool b_test = idx_b + 1 == end_b;
          if (a_test && b_test) break;
          
          if (!a_test && (b_test || points[idx_a + 1].time_point < points[idx_b + 1].time_point))
          {
            faces.push_back({ idx_a, idx_a + 1, idx_b });
            ++idx_a;
          }

          else
          {
            faces.push_back({ idx_b, idx_b + 1, idx_a });
            ++idx_b;
          }
        }
      };

      make_geometry_between_outlines(outline.first_start, outline.second_start, outline.center_start, outline.end);
      make_geometry_between_outlines(outline.second_start, outline.center_start, outline.center_start, outline.end);
    }   

    void create_base_geometry(const std::vector<OutlinePoint>& outline_points, OutlineIndices outline_indices,
                              const resources::PathStyle& path_style, float inv_max_width,
                              std::vector<PathVertex>& vertices, std::vector<PathFace>& faces)
    {
      create_base_geometry(outline_points, outline_indices, faces);

      for (auto idx = outline_indices.first_start; idx != outline_indices.end; ++idx)
      {
        PathVertex v;
        v.position = outline_points[idx].point;

        auto m = outline_points[idx].width * inv_max_width;
        if (idx >= outline_indices.center_start)
        {
          v.texture_coords = { 0.0f, 1.0f - m };
          v.z = m;
        }

        else
        {
          v.texture_coords = { m, 1.0f - m };
          v.z = 0.0f;
        }

        v.color = path_style.color;
        vertices.push_back(v);
      }
    }

    void create_path_texture_image(sf::Image& image, float max_width, float border_width, float scale)
    {
      auto image_size = next_power_of_two(static_cast<int>(max_width * scale));

      image.create(image_size, image_size, sf::Color(0, 0, 0, 0));

      for (auto y = 0; y < image_size; ++y)
      {
        auto w = image_size - y;
        auto base_width = w - border_width * scale;
        auto p = static_cast<int>(base_width);
        
        for (auto x = 0; x < p; ++x)
        {
          image.setPixel(x, y, sf::Color(255, 0, 0, 255));
        }

        if (p >= 0)
        {
          auto frac = base_width - std::floor(base_width);
          image.setPixel(p, y, sf::Color(static_cast<std::uint8_t>(255.0f * (1.0f - frac)),
                                         static_cast<std::uint8_t>(255.0f * frac), 0, 255));
        }

        for (auto x = std::max(p + 1, 0); x < w; ++x)
        {
          image.setPixel(x, y, sf::Color(0, 255, 0, 255));
        }
      }
    }

    void create_path_geometry(const resources::TrackPath& path, const resources::PathStyle& path_style,
                              float tolerance, sf::Image& path_texture,
                              std::vector<PathVertex>& vertices, std::vector<PathFace>& faces)
    {
      if (path.nodes.size() >= 2)
      {
        std::vector<OutlinePoint> outline_points;

        OutlineProperties properties;
        properties.center_line = false;
        properties.tolerance = tolerance;
        properties.use_relative_width = path_style.relative_width;
        properties.width = path_style.width;

        OutlineIndices indices{};
        indices.first_start = static_cast<std::uint32_t>(outline_points.size());
        generate_path_outline(path, properties, outline_points);

        indices.second_start = static_cast<std::uint32_t>(outline_points.size());
        generate_path_outline(path, invert(properties), outline_points);

        properties.center_line = true;
        properties.tolerance = 0.5; // lower tolerance because it will get surrounded by geometry anyway
        indices.center_start = static_cast<std::uint32_t>(outline_points.size());
        generate_path_outline(path, properties, outline_points);

        indices.end = static_cast<std::uint32_t>(outline_points.size());
        indices.closed = path.closed;

        faces.clear();
        vertices.clear();

        auto max_width = 0.0f;
        for (const auto& node : path.nodes)
        {
          auto width = properties.use_relative_width ?
            node.width * properties.width :
            node.width + properties.width;

          if (width > max_width)
          {
            max_width = width;
          }
        }

        auto scale = 2.0f;
        create_path_texture_image(path_texture, max_width, 1.5f, scale);

        auto inv_max_width = scale / path_texture.getSize().y;
        create_base_geometry(outline_points, indices, path_style, inv_max_width, vertices, faces);
      }
    }
  }
}
