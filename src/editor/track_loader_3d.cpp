/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "track_loader_3d.hpp"

#include "resources/include_path.hpp"

#include "core/config.hpp"

#include "utility/stream_utilities.hpp"
#include "utility/string_utilities.hpp"

#include <boost/filesystem.hpp>


namespace ts
{
  namespace resources_3d
  {
    namespace detail
    {
      struct ReaderContext
      {
        boost::string_ref file_name;
        boost::string_ref working_directory;
        std::size_t inclusion_depth = 0;
        std::size_t line_index = 0;
        std::size_t line_count = 0;
        const boost::string_ref* lines;
      };

      struct LineData
      {
        boost::string_ref line;
        boost::string_ref directive_view;
        boost::string_ref remainder; // part after directive
        
        std::string directive;
      };

      auto begin(const ReaderContext& context)
      {
        return context.lines;
      }

      auto end(const ReaderContext& context)
      {
        return context.lines + context.line_count;
      }

      const std::string& read_directive(LineData& line_data, boost::string_ref line)
      {
        line_data.line = line;
        line_data.directive_view = extract_word(line);
        line_data.directive.assign(line_data.directive_view.begin(), line_data.directive_view.end());

primitive_tolower(line_data.directive);
return line_data.directive;
      }

      static void read_height_map(Track& track, boost::string_ref height_map_file,
                                  boost::string_ref working_directory);
      static void load_track_components(Track& track, ReaderContext& context);

      static boost::filesystem::path resolve_asset_path(boost::string_ref file_name,
                                                        boost::string_ref working_directory);
    }

    static auto broken_track(const std::string& file_name)
    {
      return std::runtime_error("broken track: missing file '" + file_name + "'");
    }

    static auto broken_track(boost::string_ref file_name)
    {
      return broken_track(std::string(file_name.begin(), file_name.end()));
    }

    void TrackLoader::include(const std::string& file_name)
    {
      working_directory_ = boost::filesystem::path(file_name).parent_path().string();

      include(file_name, 0);
    }

    void TrackLoader::include(const std::string& file_name, std::size_t inclusion_depth)
    {
      auto stream = make_ifstream(file_name);
      if (!stream)
      {
        throw broken_track(file_name);
      }

      auto file_contents = read_stream_contents(stream);

      std::vector<boost::string_ref> lines;
      split_by_line(file_contents.data(), file_contents.size(), std::back_inserter(lines));

      detail::ReaderContext context;
      context.file_name = file_name;
      context.working_directory = working_directory_;
      context.inclusion_depth = inclusion_depth;
      context.line_index = 0;
      context.lines = lines.data();

      load_track_components(track_, context);
    }

    void detail::load_track_components(Track& track, ReaderContext& context)
    {
      detail::LineData line_data;
      auto line_it = begin(context), line_end = end(context);

      for (; line_it != line_end && line_data.directive != "end"; ++line_it, ++context.line_index)
      {
        const auto& directive = read_directive(line_data, *line_it);
        if (directive == "size")
        {
          Vector3u size;
          if (ArrayStream(line_data.remainder) >> size.x >> size.y >> size.z)
          {
            track.resize(size);
          }
        }

        else if (directive == "heightmap")
        {
          detail::read_height_map(track, line_data.remainder, context.working_directory);
        }
      }
    }

    void detail::read_height_map(Track& track, boost::string_ref height_map_file,
                                 boost::string_ref working_directory)
    {
      auto path = resolve_asset_path(height_map_file, working_directory);
      auto stream = make_ifstream(path.string());
      if (!stream)
      {
        throw broken_track(height_map_file);
      }

      auto file_contents = read_stream_contents(stream);
      std::vector<boost::string_ref> lines;
      split_by_line(file_contents.data(), file_contents.size(), std::back_inserter(lines));

      Vector2u size;
      std::uint32_t cell_size = 16;

      auto line_it = lines.begin();
      if (line_it != lines.end())
      {
        std::uint32_t x, y, temp_cell_size;

        if (ArrayStream(*line_it) >> x >> y >> temp_cell_size)
        {
          size = { x, y };
          cell_size = temp_cell_size;
        }        
      }

      std::uint32_t num_cells = size.x * size.y;
      std::vector<float> data(num_cells, 0.0f);
      for (std::uint32_t idx = 0; line_it != lines.end() && idx != num_cells; ++line_it, ++idx)
      {
        float z;
        if (ArrayStream(*line_it) >> z)
        {
          data[idx] = z;
        }
      }

      track.update_height_map(HeightMap(size, cell_size, std::move(data)));
    }

    boost::filesystem::path detail::resolve_asset_path(boost::string_ref file_name,
                                                       boost::string_ref working_directory)
    {
      // Working directory takes precedence over data directory.
      return resources::find_include_path(file_name, { working_directory, config::data_directory });
    }

    Track TrackLoader::get_result()
    {
      return std::move(track_);
    }
  }
}