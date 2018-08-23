/**
** Top down racing game, codenamed TS3D
** Copyright © 2017 Martijn Nijhuis (mniehoes@gmail.com)
**
** This is proprietary software: modifying or distributing this source file
** in any shape or form is not allowed without explicit permission from the author.
**/

#include "path_geometry.hpp"

#include "utility/math_utilities.hpp"
#include "utility/interpolate.hpp"

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
    };

    namespace detail
    {
      OutlinePoint outline_point_at(const resources::TrackPathNode& a, const resources::TrackPathNode& b, 
                                    float time_point, const OutlineProperties& props)
      {
        auto width = resources::path_width_at(a, b, time_point) + props.width;
        width *= 0.5f;

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

    void generate_path_segment_outline(const resources::SubPath& path, const OutlineProperties& properties,
                                       std::vector<OutlinePoint>& outline_points)
    {
      resources::StrokeSegment seg;
      seg.start_time_point = 0.0f;
      seg.end_time_point = path.nodes.size();
      generate_path_segment_outline(path, seg, properties, outline_points);
    }

    void generate_path_segment_outline(const resources::SubPath& path, const resources::StrokeSegment& segment,
                                       const OutlineProperties& properties,
                                       std::vector<OutlinePoint>& outline_points)
    {      
      auto node_count = static_cast<std::uint32_t>(path.nodes.size());
      if (node_count >= 2)
      {
        auto start_time = segment.start_time_point;
        auto end_time = segment.end_time_point;
        if (end_time < start_time)
        {
          if (path.closed) end_time += static_cast<float>(node_count);          
          else end_time = static_cast<float>(node_count - 1);
        }

        if (!path.closed)
        {
          end_time = std::min(static_cast<float>(node_count - 1), end_time);
        }        

        auto start_idx = static_cast<std::uint32_t>(start_time);
        auto end_idx = static_cast<std::uint32_t>(end_time);       

        for (auto idx = start_idx; idx <= end_idx; ++idx)
        {
          auto a = idx;
          if (a >= node_count) a -= node_count;

          auto b = idx + 1;
          while (b >= node_count) b -= node_count;
          
          auto base_t = static_cast<float>(idx);

          auto t1 = std::max(0.0f, start_time - base_t);
          auto t2 = std::min(1.0f, end_time - base_t);

          auto start = detail::outline_point_at(path.nodes[a], path.nodes[b], t1, properties);
          auto end = detail::outline_point_at(path.nodes[a], path.nodes[b], t2, properties);

          if (t2 - t1 < 0.5f)
          {
            detail::split_outline_segment(path.nodes[a], path.nodes[b], start, end, 
                                          properties, base_t, outline_points);
          }

          else
          {            
            auto mid = detail::outline_point_at(path.nodes[a], path.nodes[b], (t1 + t2) * 0.5f, properties);

            detail::split_outline_segment(path.nodes[a], path.nodes[b], start, mid, 
                                          properties, base_t, outline_points);
            detail::split_outline_segment(path.nodes[a], path.nodes[b], mid, end,
                                          properties, base_t, outline_points);
          }          

          if (idx == end_idx)
          {
            if (t2 - t1 > 0.5f || magnitude_squared(start.point - end.point) >= properties.tolerance * properties.tolerance)
            {
              end.time_point += base_t;
              outline_points.push_back(end);
            }
          }
        }
      }
    }

    auto construct_geometry_between_outlines(const std::vector<OutlinePoint>& points,
                                             std::uint32_t idx_a, std::uint32_t end_a,
                                             std::uint32_t idx_b, std::uint32_t end_b,
                                             std::vector<PathFace>& faces)
    {
      while (true)
      {
        bool a_test = idx_a + 1 >= end_a;
        bool b_test = idx_b + 1 >= end_b;
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
    
    void create_base_geometry(const std::vector<OutlinePoint>& points,
                              OutlineIndices outline,
                              std::vector<PathFace>& faces)
    {
      if (outline.center_start != outline.end)
      {
        construct_geometry_between_outlines(points, outline.first_start, outline.second_start,
                                            outline.center_start, outline.end, faces);
        construct_geometry_between_outlines(points, outline.second_start, outline.center_start,
                                            outline.center_start, outline.end, faces);
      }

      else
      {
        construct_geometry_between_outlines(points, outline.first_start, outline.second_start,
                                            outline.second_start, outline.end, faces);
      }
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

        //v.color = path_style.color;
        vertices.push_back(v);
      }
    }

    void create_base_geometry_directional(const std::vector<OutlinePoint>& outline_points, OutlineIndices outline_indices,
                                          const resources::PathStyle& path_style, float inv_texture_tile_size,
                                          std::vector<PathVertex>& vertices, std::vector<PathFace>& faces)
    {      
      if (outline_indices.second_start <= outline_indices.first_start) return;

      create_base_geometry(outline_points, outline_indices, faces);
      vertices.resize(vertices.size() + outline_indices.end - outline_indices.first_start);

      auto center_it = outline_points.begin() + outline_indices.center_start;
      auto center_end = outline_points.begin() + outline_indices.end;
      
      auto second_it = outline_points.begin() + outline_indices.second_start;
      auto second_end = center_it;
      
      auto first_it = outline_points.begin() + outline_indices.first_start;
      auto first_end = second_it;
      auto last_it = first_it;     

      auto x_offset = 0.0f;

      auto first_idx = outline_indices.first_start;
      auto second_idx = outline_indices.second_start;
      auto center_idx = outline_indices.center_start;

      {
        PathVertex v{};
        v.position = first_it->point;
        v.texture_coords = { x_offset, 0.0f };
        vertices[first_idx++] = v;
      }

      for (++first_it; first_it != first_end; ++first_it)
      {        
        auto dist = magnitude(last_it->point - first_it->point) * inv_texture_tile_size;
        auto new_x_offset = x_offset + dist;       

        const auto& p1 = *first_it;
        PathVertex v{};        
        v.position = p1.point;        
        v.texture_coords = { new_x_offset, 0.0f };
        vertices[first_idx++] = v;

        auto last = std::next(first_it) == first_end;
        for (; second_it != second_end && (second_it->time_point <= p1.time_point || last); ++second_it)
        {
          const auto& p2 = *second_it;
          auto f = (p2.time_point - last_it->time_point) / (p1.time_point - last_it->time_point);
          PathVertex v{};
          v.position = p2.point;
          v.texture_coords = { x_offset + f * dist, 1.0f };
          vertices[second_idx++] = v;
        }

        for (; center_it != center_end && (center_it->time_point <= p1.time_point || last); ++center_it)
        {
          const auto& p = *second_it;
          auto f = (p.time_point - last_it->time_point) / (p1.time_point - last_it->time_point);
          PathVertex v{};
          v.position = p.point;
          v.texture_coords = { x_offset + f * dist, 0.5f };
          vertices[center_idx++] = v;          
        }

        last_it = first_it;
        x_offset = new_x_offset;
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

    float max_path_width(const resources::TrackPath& path, float base_width)
    {
      auto max_width = 0.0f;
      for (const auto& sub_path : path.sub_paths)
      {
        if (sub_path.nodes.size() < 2) continue;

        for (const auto& node : sub_path.nodes)
        {
          auto width = node.width + base_width;

          if (width > max_width) max_width = width;
        }
      }

      return max_width;
    }

    void fade_outline(std::vector<OutlinePoint>& points, std::size_t start_idx, std::size_t end_idx, float fade_length)
    {
      if (end_idx - start_idx < 2) return;

      std::uint32_t first_fade_index = start_idx;
      std::uint32_t second_fade_index = end_idx;

      float first_fade_time = 0.0f;
      float second_fade_time = 0.0f;

      auto total_length = 0.0f;
      for (auto idx = start_idx, next = idx + 1; next < end_idx; ++idx, ++next)
      {
        auto m = magnitude(points[idx].point - points[next].point);
        if (m + total_length >= fade_length)
        {
          first_fade_time = (fade_length - total_length) / m;
          first_fade_index = idx;
          break;
        }

        total_length += m;
      }

      total_length = 0.0f;
      for (auto idx = end_idx - 1, next = idx - 1; idx > start_idx; --idx, --next)
      {
        auto m = magnitude(points[idx].point - points[next].point);
        if (m + total_length >= fade_length)
        {
          second_fade_time = (fade_length - total_length) / m;
          second_fade_index = idx;
          break;
        }

        total_length += m;
      }

      const auto& a1 = points[first_fade_index];
      const auto& b1 = points[first_fade_index + 1];
      const auto& a2 = points[second_fade_index];
      const auto& b2 = points[second_fade_index - 1];

      auto interpolate_point = [](const OutlinePoint& a, const OutlinePoint& b, float t)
      {
        OutlinePoint result;
        result.point = interpolate_linearly(a.point, b.point, t);
        result.time_point = interpolate_linearly(a.time_point, b.time_point, t);
        result.width = interpolate_linearly(a.width, b.width, t);
        result.normal = interpolate_linearly(a.normal, b.normal, t);
        return result;
      };

      auto p1 = interpolate_point(a1, b1, first_fade_time);
      auto p2 = interpolate_point(a2, b2, second_fade_time);
      if (first_fade_index + first_fade_time > second_fade_index - second_fade_time)
      {
        points[start_idx] = interpolate_point(p1, p2, 0.5f);
        points.erase(points.begin() + start_idx + 1, points.begin() + end_idx);
      }

      else
      {        
        points.erase(points.begin() + second_fade_index, points.begin() + end_idx);
        points.push_back(p2);

        points[first_fade_index] = p1;
        points.erase(points.begin() + start_idx, points.begin() + first_fade_index);
      }
    }

    void create_path_geometry(const resources::TrackPath& path, const resources::PathStyle& path_style,
                              float tolerance, sf::Image& path_texture,
                              std::vector<PathVertex>& vertices, std::vector<PathFace>& faces)
    {
      auto inv_max_width = 0.0f;
      if (path_style.texture_mode != path_style.Directional)
      {
        auto texture_scale = 8.0f;
        auto max_width = max_path_width(path, path_style.width);
        while (texture_scale * max_width >= 1024.0f)
        {
          texture_scale *= 0.5f;
        }
        create_path_texture_image(path_texture, max_width, path_style.border_width, texture_scale);
        inv_max_width = texture_scale / path_texture.getSize().y;
      }      

      faces.clear();
      vertices.clear();
      std::vector<OutlinePoint> outline_points;

      OutlineProperties props;
      props.tolerance = tolerance;
      props.width = path_style.width;

      auto inv_texture_tile_size = path_style.border_only ? 
        1.0f / path_style.border_texture_tile_size.x :
        1.0f / path_style.base_texture_tile_size.x;

      auto build_geometry = [&](OutlineIndices indices)
      {
        if (path_style.texture_mode != path_style.Directional)
        {
          create_base_geometry(outline_points, indices, path_style, inv_max_width, vertices, faces);
        }

        else
        {
          create_base_geometry_directional(outline_points, indices, path_style, inv_texture_tile_size, vertices, faces);
        }
      };

      if (path_style.border_only)
      {
        auto& inner = props;
        auto outer = props;
        outer.width += path_style.border_width * 2.0f;

        auto generate_segment = [&](const resources::StrokeSegment& seg)
        {
          if (seg.sub_path_id >= path.sub_paths.size()) return;

          auto& sub_path = path.sub_paths[seg.sub_path_id];          
          OutlineIndices indices{};
          indices.first_start = static_cast<std::uint32_t>(outline_points.size());

          generate_path_segment_outline(sub_path, seg, seg.side == seg.First ? inner : invert(inner), outline_points);
          indices.second_start = static_cast<std::uint32_t>(outline_points.size());

          generate_path_segment_outline(sub_path, seg, seg.side == seg.First ? outer : invert(outer), outline_points);
          if (path_style.fade_length >= 0.5f)
          {
            fade_outline(outline_points, indices.second_start, outline_points.size(), path_style.fade_length);
          }

          indices.center_start = static_cast<std::uint32_t>(outline_points.size());
          indices.end = indices.center_start;

          build_geometry(indices);
        };

        if (path_style.is_segmented)
        {
          for (auto& seg : path_style.segments)
          {
            generate_segment(seg);
          }
        }

        else
        {
          resources::StrokeSegment seg{};
          
          for (auto& sub_path : path.sub_paths)
          {
            seg.start_time_point = 0.0f;
            seg.end_time_point = static_cast<float>(sub_path.nodes.size()); // If not closed, will be clamped

            seg.side = seg.First;
            generate_segment(seg);

            seg.side = seg.Second;
            generate_segment(seg);

            ++seg.sub_path_id;
          }
        }
      }

      else
      {
        auto& first = props;
        auto second = invert(first);

        auto center = first;
        center.center_line = true;
        // lower tolerance because it will get surrounded by geometry anyway
        center.tolerance = 0.5f;
        

        auto generate_segment = [&](const resources::StrokeSegment& seg)
        {
          if (seg.sub_path_id >= path.sub_paths.size()) return;
          auto& sub_path = path.sub_paths[seg.sub_path_id];

          OutlineIndices indices{};
          indices.first_start = static_cast<std::uint32_t>(outline_points.size());
          generate_path_segment_outline(sub_path, seg, first, outline_points);

          indices.second_start = static_cast<std::uint32_t>(outline_points.size());
          generate_path_segment_outline(sub_path, seg, second, outline_points);

          indices.center_start = static_cast<std::uint32_t>(outline_points.size());
          generate_path_segment_outline(sub_path, seg, center, outline_points);

          indices.end = static_cast<std::uint32_t>(outline_points.size());
          build_geometry(indices);
        };

        if (path_style.is_segmented)
        {
          for (auto& seg : path_style.segments)
          {
            generate_segment(seg);
          }
        }

        else
        {
          resources::StrokeSegment seg{};
          for (auto& sub_path : path.sub_paths)
          {
            seg.start_time_point = 0.0f;
            seg.end_time_point = static_cast<float>(sub_path.nodes.size());
            seg.side = seg.First;
            generate_segment(seg);
            ++seg.sub_path_id;
          }
        }
      }
    }
  }
}
