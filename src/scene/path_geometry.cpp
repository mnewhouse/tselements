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
          props.width;

        auto normal = resources::path_normal_at(a, b, time_point);
        if (props.invert_normal) normal = -normal;

        return
        {
          time_point,
          resources::path_point_at(a, b, time_point) + normal * std::max(width, 0.0f),
          normal
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
        auto a = normalize(flip_orientation(point.point - prev.point));
        auto b = normalize(flip_orientation(next.point - point.point));

        a.x = -a.x;
        b.x = -b.x;

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

      adjust_outline_normals(outline_points, outline_indices, path.closed);
      

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


    RenderedPath render_path(const std::vector<OutlinePoint>& path_outline, const OutlineIndices& indices,
                             Vector2i track_size, int source_cell_size, int dest_cell_size,
                             int num_samples = 1)
    {
      if (indices.end == indices.partition || indices.partition == indices.start) return{};

      auto inv_source_cell_size = 1.0 / source_cell_size;
      auto num_rows = (track_size.y + source_cell_size - 1) / source_cell_size;
      auto num_columns = (track_size.x + source_cell_size - 1) / source_cell_size;

      struct EdgeInfo
      {
        double slope;
        double base;
        double min_y;
        double max_y;
        double avg_x;
        bool open;
        bool border;
      };

      std::vector<EdgeInfo> edge_info;
      std::vector<std::pair<std::int32_t, std::uint32_t>> edge_rows;

      auto compute_edge_info = [&](const OutlinePoint& a, const OutlinePoint& b)
      {
        auto start_row = static_cast<std::int32_t>(std::floor(a.point.y * inv_source_cell_size));
        auto end_row = static_cast<std::int32_t>(std::floor(b.point.y * inv_source_cell_size));

        auto diff = b.point - a.point;
        if (std::abs(diff.y) != 0.0f)
        {
          EdgeInfo info;
          info.slope = diff.x / diff.y;
          info.min_y = std::min(a.point.y, b.point.y);
          info.max_y = std::max(a.point.y, b.point.y);
          info.avg_x = (a.point.x + b.point.x) * 0.5;

          // x = slope*y + base
          // x - slope*y = base
          info.base = a.point.x - info.slope * a.point.y;
          info.border = false;
          info.open = (cross_product(b.point - a.point, make_vector2(1.0f, 0.0f)) < 0.0f) ==
            (cross_product(b.point - a.point, -a.normal) < 0.0f);

          auto edge_idx = static_cast<std::uint32_t>(edge_info.size());
          edge_info.push_back(info);

          if (end_row < start_row) std::swap(start_row, end_row);

          start_row = clamp(start_row, 0, num_rows);
          end_row = clamp(end_row, 0, num_rows);

          for (auto row = start_row; row <= end_row; ++row)
          {
            edge_rows.push_back({ row, edge_idx });
          }
        }
      };

      for (auto idx = indices.start + 1; idx < indices.partition; ++idx)
      {
        compute_edge_info(path_outline[idx - 1], path_outline[idx]);
      }

      for (auto idx = indices.partition + 1; idx < indices.end; ++idx)
      {
        compute_edge_info(path_outline[idx - 1], path_outline[idx]);
      }

      if (!indices.closed)
      {
        compute_edge_info(path_outline[indices.start], path_outline[indices.partition]);
        compute_edge_info(path_outline[indices.partition - 1], path_outline[indices.end - 1]);
      }

      std::sort(edge_rows.begin(), edge_rows.end(),
                [&](const auto& a, const auto& b)
      {
        if (a.first == b.first)
        {
          return edge_info[a.second].avg_x < edge_info[b.second].avg_x;
        }

        return a.first < b.first;
      });

      struct RenderPoint
      {
        float x;

        bool is_border;
        bool is_open;
      };

      auto scale = static_cast<float>(dest_cell_size * num_samples) / source_cell_size;
      auto inv_scale = 1.0 / scale;

      std::vector<RenderPoint> render_points;
      std::vector<RenderedPath::Pixel> pixels;

      auto dest_size = make_vector2(num_columns, num_rows) * dest_cell_size * num_samples;
      pixels.resize(dest_size.x * dest_size.y);

      RenderedPath result;
      for (auto it = edge_rows.begin(); it != edge_rows.end(); )
      {
        auto row = it->first;
        auto source_y = row * source_cell_size + 0.5 * inv_scale;

        auto range_end = std::find_if(std::next(it), edge_rows.end(), [=](const auto& v)
        {
          return v.first != row;
        });

        for (auto y = 0; y < dest_cell_size * num_samples; ++y, source_y += inv_scale)
        {
          render_points.clear();
          for (auto edge_it = it; edge_it != range_end; ++edge_it)
          {
            auto& edge = edge_info[edge_it->second];
            if (source_y < edge.min_y || source_y > edge.max_y) continue;

            RenderPoint point;
            point.x = source_y * edge.slope + edge.base;
            point.is_open = edge.open;
            point.is_border = edge.border;
            render_points.push_back(point);
          }

          // Every pixel in the scanline for which there are more open render_points before it than closed
          // ones is is filled.
          if (render_points.size() >= 2)
          {
            // Should already be mostly sorted.
            std::sort(render_points.begin(), render_points.end(),
                      [](const RenderPoint& a, const RenderPoint& b)
            {
              return a.x < b.x;
            });

            std::int32_t border_counter = 0;
            std::int32_t base_counter = 0;

            auto dest_y = row * num_samples * dest_cell_size + y;
            auto row_ptr = &pixels[dest_y * dest_size.x];

            auto it = render_points.begin(), next_it = std::next(it);
            for (; next_it != render_points.end(); ++it, ++next_it)
            {
              if (base_counter == 0) ++base_counter;
              else --base_counter;

              if (it->is_open)
              {
                if (it->is_border) ++border_counter;
                else ++base_counter;
              }

              else
              {
                if (it->is_border) --border_counter;
                else --base_counter;
              }

              auto start_x = static_cast<std::int32_t>(it->x * scale + 0.5f);
              auto end_x = static_cast<std::int32_t>(next_it->x * scale + 0.5f);

              start_x = clamp(start_x, 0, track_size.x);
              end_x = clamp(end_x, 0, track_size.x);

              // Fill pixels
              if (base_counter > 0)
              {
                for (auto x = start_x; x < end_x; ++x)
                {
                  row_ptr[x].b = 255;
                }
              }

              else if (border_counter > 0)
              {
                for (auto x = start_x; x < end_x; ++x)
                {
                  row_ptr[x].e = 255;
                }
              }
            }
          }
        }

        it = range_end;
      }

      sf::Image image;
      image.create(dest_size.x, dest_size.y);

      auto p = pixels.data();
      for (auto y = 0; y < dest_size.y; ++y)
      {
        for (auto x = 0; x < dest_size.x; ++x, ++p)
        {          
          image.setPixel(x, y, sf::Color(p->b, p->e, 0, 255));
        }
      }

      image.saveToFile("cunt.png");
      return{};
    }

    void create_base_geometry(const std::vector<OutlinePoint>& outline_points, OutlineIndices outline_indices,
                              const resources::PathStyle& path_style, Vector2f texture_size,
                              std::vector<PathVertex>& vertices, std::vector<PathFace>& faces)
    {
      create_base_geometry(outline_points, outline_indices, faces);

      auto inv_texture_scale = path_style.texture_scale / texture_size;
      for (auto idx = outline_indices.start; idx != outline_indices.end; ++idx)
      {
        PathVertex v;
        v.position = outline_points[idx].point;
        v.texture_coords = v.position * inv_texture_scale;
        v.color = path_style.color;
        vertices.push_back(v);
      }

      render_path(outline_points, outline_indices, { 640, 400 }, 16, 16, 1);
    }

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
  }
}
