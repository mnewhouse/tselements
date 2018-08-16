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
      std::uint32_t start, partition, end;
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

        return
        {
          time_point,
          resources::path_point_at(a, b, time_point) + normal * std::max(width, 0.0f),
          normal,
          width
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

          if (check_direction())
          {
            // We're within acceptable parameters, add the point to the array and don't recurse any further.   

            outline_points.push_back(start);
            outline_points.back().time_point += base_time_point;
          }

          else
          {
            // We have a malformed path, we need to find and add a point that's just before
            // where it goes wrong.
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

        // Insert the final vertex
        PathVertex v{};
        outline_points.push_back(detail::outline_point_at(*node_it, *next_it, 1.0f, outline_properties));
        outline_points.back().time_point = static_cast<double>(segment_id); 
      }
    }
    
    void create_base_geometry(const std::vector<OutlinePoint>& points,
                              OutlineIndices outline,
                              std::vector<PathFace>& faces)
    {
      auto first_index = outline.start;
      auto second_index = outline.partition;

      while (first_index + 1 < outline.partition || second_index + 1 < outline.end)
      {
        bool first_end = first_index + 1 == outline.partition;
        bool second_end = second_index + 1 == outline.end;

        if (!first_end && (second_end || points[first_index + 1].time_point < points[second_index + 1].time_point))
        {          
          faces.push_back({ first_index, first_index + 1, second_index });
          ++first_index;
        }

        else
        {
          faces.push_back({ second_index, second_index + 1, first_index });
          ++second_index;
        }
      }
    }   

    void adjust_outline_normals(std::vector<OutlinePoint>& outline_points, OutlineIndices outline_indices, bool closed)
    {
      auto begin = outline_points.begin() + outline_indices.start;
      auto partition = outline_points.begin() + outline_indices.partition;
      auto end = outline_points.begin() + outline_indices.end;

      auto fix_normal = [](OutlinePoint& point, const OutlinePoint& prev, const OutlinePoint& next)
      {
        auto a = normalize(tangent(point.point - prev.point));
        auto b = normalize(tangent(next.point - point.point));

        if (dot_product(a, point.normal) < 0.0f) a = -a;
        if (dot_product(b, point.normal) < 0.0f) b = -b;

        auto normal = (a + b) * 0.5f;

        if (dot_product(normal, point.normal) < 0.0f) normal = -normal;

        auto dp = dot_product(normal, point.normal);
        if (dp >= 0.5f)
        {
          normal /= dp;
        }

        point.normal = normal;
      };

      if (std::distance(begin, partition) >= 2 && std::distance(partition, end) >= 2)
      {
        if (!closed)
        {
          //fix_normal(begin[0], partition[0], begin[1]);
          //fix_normal(partition[-1], partition[-2], end[-1]);
          //fix_normal(partition[0], begin[0], partition[1]);
          //fix_normal(end[-1], end[-2], partition[-1]);
        }

        else
        {
          fix_normal(begin[0], partition[-2], begin[1]);
          fix_normal(partition[-1], partition[-2], begin[1]);
          fix_normal(partition[0], end[-2], partition[1]);
          fix_normal(end[-1], end[-2], partition[1]);
        }

        for (auto it = begin + 1; it < partition - 1; ++it)
        {
          fix_normal(it[0], it[-1], it[1]);
        }

        for (auto it = partition + 1; it < end - 1; ++it)
        {
          fix_normal(it[0], it[-1], it[1]);
        }
      }
    }

    OutlineIndices generate_path_outline(const resources::TrackPath& path, const resources::PathStyle& path_style,
                                         const OutlineProperties& first_outline, const OutlineProperties& second_outline,
                                         std::vector<OutlinePoint>& outline_points)
    {
      OutlineIndices outline_indices;
      outline_indices.closed = path.closed;
      outline_indices.start = static_cast<std::uint32_t>(outline_points.size());

      generate_path_outline(path, first_outline, outline_points);
      outline_indices.partition = static_cast<std::uint32_t>(outline_points.size());

      generate_path_outline(path, second_outline, outline_points);
      outline_indices.end = static_cast<std::uint32_t>(outline_points.size());

      //adjust_outline_normals(outline_points, outline_indices, path.closed);
      return outline_indices;
    }

    OutlineIndices generate_path_outline(const resources::TrackPath& path, const resources::PathStyle& path_style,
                                         float tolerance,
                                         std::vector<OutlinePoint>& outline_points)
    {
      OutlineProperties properties;
      properties.tolerance = tolerance;
      properties.use_relative_width = path_style.relative_width;
      properties.width = path_style.width;
      return generate_path_outline(path, path_style, properties, invert(properties), outline_points);
    }

    void create_base_geometry(const std::vector<OutlinePoint>& outline_points, OutlineIndices outline_indices,
                              const resources::PathStyle& path_style, float inv_max_width,
                              std::vector<PathVertex>& vertices, std::vector<PathFace>& faces)
    {
      create_base_geometry(outline_points, outline_indices, faces);

      for (auto idx = outline_indices.start; idx != outline_indices.end; ++idx)
      {
        PathVertex v;
        v.position = outline_points[idx].point;

        auto m = outline_points[idx].width * inv_max_width;
        if (idx >= outline_indices.partition) v.texture_coords = { -m, 1.0f - m };
        else v.texture_coords = { m, 1.0f - m };

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
      std::vector<OutlinePoint> outline_points;

      OutlineProperties properties;
      properties.tolerance = tolerance;
      properties.use_relative_width = path_style.relative_width;
      properties.width = path_style.width;
      auto outline_indices = generate_path_outline(path, path_style, properties, 
                                                   invert(properties), outline_points);

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
      create_base_geometry(outline_points, outline_indices, path_style, inv_max_width, vertices, faces);
    }
    /*
    void create_border_geometry(const std::vector<OutlinePoint>& outline, OutlineIndices outline_indices,
                                const resources::BorderStyle& border_style, Vector2f texture_size,
                                std::vector<PathVertex>& vertices, std::vector<PathFace>& faces)
    {
      auto inv_texture_scale = border_style.texture_scale / texture_size;
      auto make_vertex = [=](Vector2f point)
      {
        PathVertex v;
        v.position = point;
        v.texture_coords = point * inv_texture_scale;
        v.color = border_style.color;
        return v;
      };

      auto create_border = [&](auto start_idx, auto end_idx)
      {
        std::uint32_t a = static_cast<std::uint32_t>(vertices.size());
        for (auto idx = start_idx; idx != end_idx; ++idx)
        {
          auto& p = outline[idx];

          auto inner_vtx = make_vertex(p.point + p.normal * border_style.offset);
          auto outer_vtx = make_vertex(p.point + p.normal * (border_style.offset + border_style.width));

          vertices.push_back(inner_vtx);
          vertices.push_back(outer_vtx);
        }
        
        
        for (auto idx = start_idx + 1; idx < end_idx; ++idx, a += 2)
        {          
          faces.push_back({ a, a + 1, a + 2 });
          faces.push_back({ a + 1, a + 3, a + 2 });
        }
      };      

      create_border(outline_indices.start, outline_indices.partition);
      create_border(outline_indices.partition, outline_indices.end);
    }
    */
  }
}
