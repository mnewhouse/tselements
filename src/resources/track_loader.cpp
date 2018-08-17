/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/


#include "track_loader.hpp"
#include "tile_library.hpp"
#include "texture_library.hpp"
#include "include_path.hpp"
#include "track_layer.hpp"
#include "terrain_definition.hpp"
#include "terrain_library.hpp"

#include "core/config.hpp"

#include "utility/debug_log.hpp"
#include "utility/stream_utilities.hpp"
#include "utility/string_utilities.hpp"

#include <boost/optional.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/filesystem/path.hpp>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <iostream>
#include <fstream>

namespace ts
{
  namespace resources
  {
    BrokenTrackException::BrokenTrackException(const std::string& missing_file)
      : std::runtime_error("broken track: missing file '" + missing_file + "'")
    {
    }

    namespace io = boost::iostreams;

    // Loading context keeps track of where it's at
    struct TrackLoader::Context
    {
      boost::string_ref file_name;
      std::size_t inclusion_depth = 0;
      std::size_t line_index = 0;
      const std::vector<boost::string_ref>* lines;
    };

    using LoadingContext = TrackLoader::Context;

    boost::filesystem::path resolve_asset_path(boost::string_ref file_name, boost::string_ref working_directory)
    {
      // Working directory takes precedence over data directory.
      return find_include_path(file_name, { working_directory, config::data_directory });
    }

    // Helper function to spew an error if something went wrong
    static void insufficient_parameters(boost::string_ref directive, const LoadingContext& context)
    {
      DEBUG_RELEVANT << "Warning: insufficient parameters to '" << directive <<
        "' directive in " << context.file_name << " on line " << context.line_index << "." << debug::endl;
    }

    // This function can be used to keep reading and discarding lines until a line with an "end" directive is found.
    // Useful for error handling, or unimplemented features.
    static void skip_until_end(LoadingContext& context)
    {
      const auto& lines = *context.lines;
      auto& line_index = context.line_index;

      for (std::string directive; line_index < lines.size() && directive != "end"; ++context.line_index)
      {
        boost::string_ref line = lines[line_index];
        boost::string_ref directive_view = extract_word(line);

        // Minor optimization: read 4 characters at most
        directive.assign(directive_view.begin(), std::min<std::size_t>(directive_view.size(), 4));

        primitive_tolower(directive);
      }
    }

    template <typename Func>
    static void do_callback(boost::string_ref line, std::string& directive, Func&& callback)
    {
      auto directive_view = extract_word(line);

      directive.assign(directive_view.begin(), directive_view.end());
      primitive_tolower(directive);

      boost::string_ref remainder = make_string_ref(directive_view.end(), line.end());
      remainder = remove_leading_spaces(remainder, "\t ");

      if (!directive.empty() && directive.front() != '#')
      {
        callback(directive, directive_view, remainder);
      }      
    }

    template <typename Func>
    auto loop_until_end(LoadingContext& context, Func&& callback)
    {
      const auto& lines = *context.lines;
      auto& line_index = context.line_index;

      for (std::string directive; line_index < lines.size() && directive != "end"; ++line_index)
      {
        do_callback(lines[line_index], directive, callback);
      }
    }

    template <typename Func>
    auto loop_all(LoadingContext& context, Func&& callback)
    {
      const auto& lines = *context.lines;
      auto& line_index = context.line_index;

      for (std::string directive; line_index < lines.size(); )
      {
        auto start_index = line_index;
        do_callback(lines[line_index], directive, callback);

        if (line_index == start_index)
        {
          ++line_index;
        }
      }
    }


#define LOOP_LAMBDA [&](const std::string& directive, boost::string_ref directive_view, boost::string_ref remainder)

    static bool read_tile(LoadingContext& context, boost::string_ref directive, boost::string_ref line,
                          Tile& tile, bool level_tile = false)
    {
      TileId tile_id;
      Vector2i position;
      std::int32_t rotation;
      std::uint32_t level = 0;

      ArrayStream stream(line);
      if (level_tile) stream >> level;

      if (stream >> tile_id >> position.x >> position.y >> rotation)
      {
        tile.id = tile_id;
        tile.position = position;
        tile.rotation = rotation;
        tile.level = level;
        return true;
      }

      insufficient_parameters(directive, context);
      return false;
    }

    static void load_geometry(TrackLayer& layer, LoadingContext& context, std::uint32_t texture_id,
                              std::uint32_t level, std::size_t vertex_count)
    {
      /*
      std::size_t start_line = context.line_index;

      Geometry geometry;
      geometry.texture_id = texture_id;
      geometry.level = level;

      auto& vertices = geometry.vertices;
      vertices.reserve(vertex_count);

      const auto& lines = *context.lines;
      auto& line_index = context.line_index;

      for (std::string directive; line_index < lines.size() && directive != "end"; ++context.line_index)
      {
        boost::string_ref line = lines[line_index];
        boost::string_ref directive_view = extract_word(line);
        auto remainder = make_string_ref(directive_view.end(), line.end());

        directive.assign(directive_view.begin(), directive_view.end());
        primitive_tolower(directive);
        if (directive == "v")
        {
          Vertex vertex;
          ArrayStream stream(remainder);
          if (stream >> vertex.position.x >> vertex.position.y >>
              vertex.texture_coords.x >> vertex.texture_coords.y)
          {
            std::uint32_t r, g, b, a;

            if (stream >> r >> g >> b >> a)
            {
              vertex.color =
              {
                static_cast<std::uint8_t>(r),
                static_cast<std::uint8_t>(g),
                static_cast<std::uint8_t>(g),
                static_cast<std::uint8_t>(a)
              };
            }

            else
            {
              vertex.color = { 0xFF, 0xFF, 0xFF, 0xFF };
            }

            vertices.push_back(vertex);
          }

          else insufficient_parameters(directive_view, context);
        }
      }

      if (vertices.size() == 0)
      {
        DEBUG_RELEVANT << "Warning: geometry definition on line " << start_line <<
          " has no vertices, ignoring." << debug::endl;
      }

      else
      {
        layer.geometry().push_back(std::move(geometry));

        if (vertex_count != vertices.size())
        {
          DEBUG_RELEVANT << "Warning: vertex count does not match allocated space for geometry definition on line " <<
            start_line << "." << debug::endl;
        }
      }
      */
    }

    static void load_track_components(Track& track, LoadingContext& context)
    {
      TrackLayer* current_layer = nullptr;

      // Helper lambda to ensure that a layer with the specified level exists,
      // by creating a new layer if it doesn't.
      auto ensure_layer_exists = [&](std::uint32_t level)
      {
        if (!current_layer || current_layer->level() != level)
        {
          auto layer_count = track.layer_count();
          current_layer = track.create_layer(TrackLayerType::Tiles, "Layer " + std::to_string(layer_count + 1), level);
        }
      };

      loop_until_end(context, LOOP_LAMBDA
      {
        if (directive == "a" || directive == "leveltile")
        {
          bool level_tile = directive != "a";

          Tile tile;
          if (read_tile(context, directive_view, remainder, tile, level_tile))
          {
            ensure_layer_exists(tile.level);

            if (auto tiles = current_layer->tiles())
            {
              tiles->push_back(tile);
            }
          }
        }

        else if (directive == "tilelayer")
        {
          ArrayStream stream(remainder);

          std::uint32_t level, visible;
          std::string layer_name;

          if (stream >> level >> visible >> std::ws && std::getline(stream, layer_name))
          {
            current_layer = track.create_layer(resources::TrackLayerType::Tiles, std::move(layer_name), level);
            current_layer->set_visible(visible != 0);
          }
        }

        else if (directive == "baseterrain")
        {
          ArrayStream stream(remainder);
          std::uint32_t texture_id, terrain_id;
          
          if (stream >> terrain_id >> texture_id)
          {
            char h{};
            stream >> h;

            current_layer = track.create_layer(resources::TrackLayerType::BaseTerrain, "Base Terrain", 0);
            current_layer->set_visible(h == 'H' || h == 'h');

            auto* base_terrain = current_layer->base_terrain();
            base_terrain->texture_id = texture_id;
            base_terrain->terrain_id = terrain_id;
          }
        }
      });
    }

    static void load_tile_definitions(TileLibrary& tile_library, LoadingContext& context,
                                      boost::string_ref pattern_file, boost::string_ref image_file,
                                      boost::string_ref working_directory)
    {
      auto pattern_path = resolve_asset_path(pattern_file, working_directory);
      auto image_path = resolve_asset_path(image_file, working_directory);

      if (pattern_path.empty())
      {
        throw BrokenTrackException({ pattern_file.begin(), pattern_file.end() });
      }

      if (image_path.empty())
      {
        throw BrokenTrackException({ image_file.begin(), image_file.end() });
      }

      auto tile_def_interface = tile_library.define_tile_set(pattern_path.string(), image_path.string());

      loop_until_end(context, LOOP_LAMBDA
      {
        if (directive == "tile" || directive == "norottile")
        {
          TileId tile_id;
          IntRect pat_rect, img_rect;

          if (ArrayStream(remainder) >> tile_id >> pat_rect.left >> pat_rect.top >> pat_rect.width >> pat_rect.height >>
              img_rect.left >> img_rect.top >> img_rect.width >> img_rect.height)
          {
            tile_def_interface.define_tile(tile_id, pat_rect, img_rect);
          }

          else insufficient_parameters(directive_view, context);
        }
      });
    }

    static void load_tile_group_definitions(TileLibrary& tile_library, LoadingContext& context, TileId group_id, std::size_t group_size)
    {
      TileGroupDefinition tile_group;
      tile_group.id = group_id;

      auto& sub_tiles = tile_group.sub_tiles;
      sub_tiles.reserve(group_size);

      loop_until_end(context, LOOP_LAMBDA
      {
        if (directive == "a")
        {
          Tile tile;
          if (sub_tiles.size() < group_size && read_tile(context, directive_view, remainder, tile))
          {
            sub_tiles.push_back(tile);
          }
        }

        else if (directive == "leveltile")
        {
          Tile tile;
          if (sub_tiles.size() < group_size && read_tile(context, directive_view, remainder, tile, true))
          {
            sub_tiles.push_back(tile);
          }
        }
      });

      if (tile_group.sub_tiles.size() == 0)
      {
        DEBUG_RELEVANT << "Warning: empty tile group, ignoring. [group_id=" << group_id << "]" << debug::endl;
      }

      else
      {
        tile_library.define_tile_group(tile_group);

        if (group_size != tile_group.sub_tiles.size())
        {
          DEBUG_RELEVANT << "Warning: tile group size does not match allocated space. [group_id=" << group_id << "]" << debug::endl;
        }
      }      
    }

    static void load_collision_shape(TileLibrary& tile_library, LoadingContext& context,
                                     std::uint32_t tile_id)
    {
      CollisionShape collision_shape{};
      loop_until_end(context, LOOP_LAMBDA
      {
        if (directive == "circle")
        {
          float x, y, radius, bounciness;
          std::uint32_t height;

          ArrayStream stream(remainder);
          if (stream >> x >> y >> radius >> bounciness)
          {
            if (!(stream >> height)) height = 1;

            collision_shapes::Circle circle;
            circle.center = { x, y };
            circle.radius = radius;
            circle.height = height;

            collision_shape.sub_shapes.push_back({ circle, bounciness });
          }
        }
      });

      tile_library.define_collision_shape(tile_id, collision_shape);
    }
    
    bool process_path_style_directive(BaseStyle& style, const std::string& directive, boost::string_ref remainder)
    {
      if (directive == "basetexture")
      {
        ArrayStream(remainder) >> style.base_texture;
      }

      else if (directive == "bordertexture")
      {
        ArrayStream(remainder) >> style.border_texture;
      }

      else if (directive == "terrain")
      {
        ArrayStream(remainder) >> style.terrain_id;
      }

      else if (directive == "width")
      {
        ArrayStream(remainder) >> style.width;
      }

      else return false;

      return true;
    }

    static BorderStyle load_border_style(LoadingContext& context)
    {
      BorderStyle style;
      loop_until_end(context, LOOP_LAMBDA
      {
        process_path_style_directive(style, directive, remainder);
      });

      return style;
    }

    static PathStyle load_path_style(LoadingContext& context)
    {
      PathStyle style;
      loop_until_end(context, LOOP_LAMBDA
      {
        if (process_path_style_directive(style, directive, remainder))
        {
          // do nothing
        }

        else if (directive == "border")
        {
          style.border_styles.push_back(load_border_style(context));          
        }
      });

      return style;
    }

    static void load_terrain_definition(TerrainLibrary& terrain_library, LoadingContext& context)
    {
      boost::optional<TerrainId> terrain_id;
      TerrainDefinition terrain_def;

      loop_until_end(context, LOOP_LAMBDA
      {
        auto read_property = [&](const char* property_directive, auto& value)
        {
          if (directive == property_directive)
          {
            if (ArrayStream(remainder) >> value) return true;

            insufficient_parameters(directive_view, context);
          }

          return false;
        };

        std::uint16_t temp_id, color;
        if (read_property("id", temp_id)) { terrain_id = static_cast<TerrainId>(temp_id); }

        else if (read_property("acceleration", terrain_def.acceleration)) {}
        else if (read_property("braking", terrain_def.braking)) {}
        else if (read_property("steering", terrain_def.steering)) {}
        else if (read_property("cornering", terrain_def.cornering)) {}
        else if (read_property("traction", terrain_def.traction)) {}
        else if (read_property("roughness", terrain_def.roughness)) {}
        else if (read_property("bounciness", terrain_def.bounciness)) {}
        else if (read_property("jump", terrain_def.jump)) {}
        else if (read_property("tyremark", terrain_def.tyre_mark)) {}
        else if (read_property("skidmark", terrain_def.skid_mark)) {}
        else if (read_property("iswall", terrain_def.is_wall)) {}
        else if (read_property("red", color)) { terrain_def.color.r = static_cast<std::uint8_t>(color); }
        else if (read_property("green", color)) { terrain_def.color.g = static_cast<std::uint8_t>(color); }
        else if (read_property("blue", color)) { terrain_def.color.b = static_cast<std::uint8_t>(color); }
      });

      if (!terrain_id)
      {
        DEBUG_RELEVANT << "Terrain defined without id, ignoring..." << debug::endl;
      }

      else
      {
        terrain_def.id = *terrain_id;
        terrain_library.define_terrain(terrain_def);
      }
    }    

    static void load_control_points(Track& track, LoadingContext& context)
    {
      loop_until_end(context, LOOP_LAMBDA
      {
        if (directive == "point")
        {
          std::int32_t x, y, length, direction;

          if (ArrayStream(remainder) >> x >> y >> length >> direction)
          {
            ControlPoint point;
            point.start = { x, y };
            point.end = point.start;
            point.flags = 0;

            // 0 = horizontal, 1 = vertical.
            if (direction == 0)
            {
              point.end.y += length;
              point.type = ControlPoint::VerticalLine;
            }
            else
            {
              point.end.x += length;
              point.type = ControlPoint::HorizontalLine;
            }

            track.add_control_point(point);
          }

          else insufficient_parameters(directive_view, context);
        }

        if (directive == "check")
        {
          // Special kind of control point, which runs from one point to another.
          std::int32_t x1, y1, x2, y2;
          std::uint32_t flags = 0;

          ArrayStream stream(remainder);
          if (stream >> x1 >> y1 >> x2 >> y2)
          {
            stream >> flags;

            ControlPoint point;
            point.start = { x1, y1 };
            point.end = { x2, y2 };
            point.type = ControlPoint::FinishLine;
            point.flags = flags;
            track.add_control_point(point);
          }
        }

        else insufficient_parameters(directive_view, context);
      });
    }

    static void load_start_points(Track& track, LoadingContext& context, std::size_t point_count)
    {
      loop_until_end(context, LOOP_LAMBDA
      {
        if (point_count != 0 && directive == "point")
        {
          std::int32_t x, y, rotation; std::uint32_t level;          
          if (ArrayStream(remainder) >> x >> y >> rotation >> level)
          {
            StartPoint point;
            point.position = { x, y };
            point.rotation = rotation;
            point.level = level;

            track.add_start_point(point);
            --point_count;
          }

          else insufficient_parameters(directive_view, context);
        }
      });
    }


    void TrackLoader::load_from_file(const std::string& file_name)
    {
      // Reset the track instance with a default-constructed one
      // so we don't have any residual state.
      Track dummy_track;
      std::swap(dummy_track, track_);

      try
      {
        track_.set_path(file_name);
        working_directory_ = boost::filesystem::path(file_name).parent_path().string();

        include(file_name);
      }

      catch (...)
      {
        std::swap(dummy_track, track_);
        throw;
      }     
    }

    void TrackLoader::include(const std::string& file_name)
    {
      include(file_name, 0);
    }    

    void TrackLoader::include(const std::string& file_name, std::size_t inclusion_depth)
    {
      // If we inserted, the file didn't exist in the include file set.
      if (included_files_.insert(file_name).second)
      {
        auto stream = make_ifstream(file_name, std::ios::in);
        if (!stream)
        {
          throw BrokenTrackException(file_name);
        }

        auto file_contents = read_stream_contents(stream);

        std::vector<boost::string_ref> lines;
        split_by_line(file_contents.data(), file_contents.size(), std::back_inserter(lines));

        LoadingContext context;
        context.file_name = { file_name.data(), file_name.size() };
        context.line_index = 0;
        context.lines = &lines;
        
        load_included_file(context, inclusion_depth);
      }
    }

    

    void TrackLoader::load_included_file(Context& context, std::size_t inclusion_depth)
    {
      std::size_t num_lines = context.lines->size();

      const auto& lines = *context.lines;
      auto& line_index = context.line_index;
      loop_all(context, LOOP_LAMBDA
      {
        if (directive == "include")
        {
          if (!remainder.empty())
          {
            auto path = resolve_asset_path(remainder, working_directory_);
            include(path.string(), inclusion_depth + 1);
          }

          else insufficient_parameters(directive_view, context);
        }

        else if (directive == "controlpoints")
        { 
          load_control_points(track_, context);
        }

        else if (directive == "startpoints")
        {
          std::size_t point_count;
          if (ArrayStream(remainder) >> point_count)
          {
            load_start_points(track_, context, point_count);
          }
            
          else insufficient_parameters(directive_view, context);
        }

        else if (directive == "tiledefinition")
        {
          auto pattern_file = extract_word(remainder);
          auto image_file = extract_word(make_string_ref(pattern_file.end(), remainder.end()));

          if (!pattern_file.empty() && !image_file.empty())
          {
            load_tile_definitions(track_.tile_library(), context, pattern_file, image_file, working_directory_);
          }

          else
          {
            insufficient_parameters(directive_view, context);
            skip_until_end(context);
          }
        }

        else if (directive == "collisionshape")
        {
          std::uint32_t tile_id = 0;
          if (ArrayStream(remainder) >> tile_id)
          {
            load_collision_shape(track_.tile_library(), context, tile_id);
          }
        }

        else if (directive == "texture")
        {         
          std::uint32_t texture_id;
          std::string path;
          ArrayStream stream(remainder);
          if (stream >> texture_id >> std::ws && std::getline(stream, path))
          {
            Texture tex;
            tex.file_name = resolve_asset_path(path, working_directory_).string();
            if (tex.file_name.empty())
            {
              throw BrokenTrackException({ path.begin(), path.end() });
            }

            tex.id = texture_id;
            track_.texture_library().define_texture(std::move(tex));
          }
            
          else
          {
            insufficient_parameters(directive_view, context);
            skip_until_end(context);
          }
        }

        else if (directive == "tilegroup" || directive == "norottilegroup")
        {
          TileId group_id;
          std::size_t group_size;
          if (ArrayStream(remainder) >> group_id >> group_size)
          {
            load_tile_group_definitions(track_.tile_library(), context, group_id, group_size);
          }
            
          else
          {
            insufficient_parameters(directive_view, context);
            skip_until_end(context);
          }
        }

        else if (directive == "pathstyle")
        {
          PathStylePreset preset;            
          if (std::getline(ArrayStream(remainder), preset.name))
          {              
            preset.style = load_path_style(context);
            track_.path_library().add_style_preset(preset);
          }            
        }

        else if (directive == "terrain")
        {
          load_terrain_definition(track_.terrain_library(), context);
        }

        if (inclusion_depth == 0)
        {
          // These properties only work for the "main" file.

          if (directive == "size")
          {
            auto next_word = extract_word(remainder);
            if (next_word == "td")
            {
              auto size_data = make_string_ref(next_word.end(), remainder.end());
              std::int32_t height_levels;
              Vector2i size;
              if (ArrayStream(size_data) >> height_levels >> size.x >> size.y)
              {
                track_.set_height_level_count(height_levels);
                track_.set_size(size);
              }
            }
          }

          else if (directive == "maker")
          {
            auto author = remove_leading_spaces(remainder, "\t ");
            if (!author.empty())
            {
              track_.set_author(std::string(author.begin(), author.end()));
            }

            else insufficient_parameters(directive_view, context);
          }

          else if (directive == "a" || directive == "leveltile" || directive == "geometry" || directive == "tilelayer" ||
                    directive == "baseterrain")
          {
            // If any of these directives are reached, we only have track components to process.
            // It will loop through all the remaining lines until an 'End' is found.
            // It is then up to us to finish up here, because the loading will be finished.

            // Don't increment the line index and let it process this line again.
            load_track_components(track_, context);

            // And make sure we're finished here.
            line_index = context.lines->size();
          }
        }
      });
    }

    Track TrackLoader::get_result()
    {
      return std::move(track_);
    }

    Track load_track(boost::string_ref path)
    {
      return load_track(std::string(path.begin(), path.end()));
    }

    Track load_track(const std::string& path)
    {
      TrackLoader track_loader;
      track_loader.load_from_file(path);
      return track_loader.get_result();
    }
  }
}