/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "car_loader.hpp"
#include "pattern.hpp"
#include "collision_mask_detail.hpp"
#include "include_path.hpp"

#include "core/config.hpp"

#include "utility/rect.hpp"
#include "utility/stream_utilities.hpp"
#include "utility/string_utilities.hpp"
#include "utility/debug_log.hpp"

#include <boost/filesystem/path.hpp>

#include <algorithm>

namespace ts
{
  namespace resources
  {
    static void insufficient_parameters(boost::string_ref car_name, boost::string_ref directive)
    {
      DEBUG_RELEVANT << "Insufficient parameters for directive '" << directive << "', [car_name=" << car_name << "]" << debug::endl;
    }

    template <typename LineIt>
    static LineIt read_car_definition(boost::string_ref car_name, boost::string_ref working_directory, 
                                      LineIt line_it, LineIt lines_end, CarDefinition& car_def, PatternLoader& pattern_loader)
    {
      car_def.car_name.assign(car_name.begin(), car_name.end());

      for (std::string directive; directive != "end" && line_it != lines_end; ++line_it)
      {
        boost::string_ref trimmed_line = remove_leading_spaces(*line_it);
        boost::string_ref directive_view = extract_word(trimmed_line);

        directive.assign(directive_view.begin(), directive_view.end());
        primitive_tolower(directive);

        boost::string_ref remainder = make_string_ref(directive_view.end(), line_it->end());
        remainder = remove_leading_spaces(remainder);

        if (directive == "image" || directive == "rotimage")
        {
          std::string image_path;
          IntRect rect;
          double scale;

          if (ArrayStream(remainder) >> image_path >> rect.left >> rect.top >> rect.width >> rect.height >> scale)
          {
            auto full_image_path = find_include_path(image_path, { working_directory, config::data_directory });
            if (!full_image_path.empty())
            {
              car_def.image_path = full_image_path.string();
              car_def.image_rect = rect;
              car_def.image_scale = scale;

              if (directive == "rotimage" && car_def.num_rotations <= 1)
              {
                car_def.image_type = CarImage::Prerotated;
                car_def.num_rotations = rect.width / rect.height;
              }

              else
              {
                car_def.num_rotations = 1;
                car_def.image_type = CarImage::Default;
              }
            }

            else
            {
              DEBUG_RELEVANT << "Error loading car '" << car_name << "': image file not found" << debug::endl;
            }
          }

          else insufficient_parameters(car_name, directive_view);
        }

        else if (directive == "bounciness")
        {
          if (!(ArrayStream(remainder) >> car_def.bounciness))
          {
            insufficient_parameters(car_name, directive_view);
          }
        }

        else if (directive == "mask")
        {
          std::string pattern_path;
          IntRect rect;
          if (ArrayStream(remainder) >> pattern_path >> rect.left >> rect.top >> rect.width >> rect.height)
          {
            auto full_pattern_path = find_include_path(pattern_path, { working_directory, config::data_directory });
            if (!full_pattern_path.empty())
            {
              auto wall_test = [](std::uint8_t p)
              {
                return p != 0;
              };

              const std::uint32_t frame_count = 64;
              const auto& pattern = pattern_loader.load_from_file(full_pattern_path.string());

              car_def.collision_mask = std::make_shared<CollisionMask>(dynamic_mask, pattern, rect,
                                                                        frame_count, wall_test);
            }

            else
            {
              DEBUG_RELEVANT << "Error loading car '" << car_name << "': unable to open pattern file." << debug::endl;
            }
          }
        }

        else if (directive == "enginesample")
        {
          auto full_sound_path = find_include_path(remainder, { working_directory, config::data_directory });
          if (!full_sound_path.empty())
          {
            car_def.engine_sound_path.assign(full_sound_path.string());
          }

          else
          {
            DEBUG_RELEVANT << "Error loading car '" << car_name << "': engine sample not found" << debug::endl;
          }
        }
      }

      return line_it;
    }

    template <typename LineIt, typename OutIt>
    static std::size_t load_cars_from_line_data(LineIt begin, LineIt end, boost::string_ref working_directory, 
                                                PatternLoader& pattern_loader, OutIt out)
    {
      std::size_t count = 0;

      for (auto line_it = begin; line_it != end; )
      {
        auto current_line = line_it;

        boost::string_ref trimmed_line = remove_leading_spaces(*line_it, " \t");
        boost::string_ref directive_view = extract_word(trimmed_line);

        if (directive_view.size() == 3)
        {
          if (caseless_string(directive_view) == "car")
          {
            boost::string_ref car_name = make_string_ref(directive_view.end(), line_it->end());
            car_name = remove_leading_spaces(car_name);

            if (!car_name.empty())
            {
              CarDefinition car_def;
              line_it = read_car_definition(car_name, working_directory, std::next(line_it), end, car_def, pattern_loader);

              *out++ = car_def;
              ++count;
            }
          }
        }

        if (current_line == line_it)
        {
          ++line_it;
        }
      }

      return count;
    }

    std::size_t CarLoader::load_cars_from_stream(std::istream& stream, boost::string_ref working_directory)
    {
      auto file_contents = read_stream_contents(stream);

      std::vector<boost::string_ref> lines;
      split_by_line(file_contents.data(), file_contents.size(), std::back_inserter(lines));

      return load_cars_from_line_data(lines.begin(), lines.end(), working_directory, 
                                      pattern_loader_, std::back_inserter(car_definitions_));
    }

    std::size_t CarLoader::load_cars_from_file(const std::string& file_name)
    {
      auto stream = make_ifstream(file_name, std::ios::in);
      if (stream)
      {
        std::string parent_path = boost::filesystem::path(file_name).parent_path().string();

        return load_cars_from_stream(stream, parent_path);
      }

      return 0;
    }

    std::vector<CarDefinition> CarLoader::get_result()
    {
      return std::move(car_definitions_);
    }

    const std::vector<CarDefinition>& CarLoader::car_definitions() const
    {
      return car_definitions_;
    }
  }
}