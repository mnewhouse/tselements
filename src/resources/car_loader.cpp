/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/


#include "car_loader.hpp"
#include "pattern.hpp"
#include "handling_properties.hpp"
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
                                      LineIt line_it, LineIt lines_end, CarDefinition& car_def, PatternStore& pattern_loader)
    {
      car_def.car_name.assign(car_name.begin(), car_name.end());     

      auto& handling = car_def.handling_properties;

      enum ReaderState
      {
        Main, DownforceEffect, StressFactor, CollisionShape, CollisionPolygon, End
      } reader_state = Main;

      for (std::string directive; reader_state != End && line_it != lines_end; ++line_it)
      {
        boost::string_ref trimmed_line = remove_leading_spaces(*line_it);
        boost::string_ref directive_view = extract_word(trimmed_line);

        directive.assign(directive_view.begin(), directive_view.end());
        primitive_tolower(directive);

        boost::string_ref remainder = make_string_ref(directive_view.end(), line_it->end());
        remainder = remove_leading_spaces(remainder);

        auto read_property = [&](const char* property, auto& value)
        {
          if (directive == property)
          {
            if (ArrayStream(remainder) >> value) return true;
            else insufficient_parameters(car_name, directive_view);
          }

          return false;
        };

        auto read_property_1000 = [&](const char* property, auto& value)
        {
          auto result = read_property(property, value);
          if (result) value *= 1000;

          return result;
        };

        if (reader_state == DownforceEffect)
        {
          if (directive == "end") reader_state = Main;

          /*
          else if (read_property("tractionlimit", handling.downforce_effect.traction_limit)) {}
          else if (read_property("braking", handling.downforce_effect.braking)) {}
          else if (read_property("cornering", handling.downforce_effect.cornering)) {}
          else if (read_property("antislide", handling.downforce_effect.antislide)) {}
          else if (read_property("rollingresistance", handling.downforce_effect.rolling_resistance)) {}
          */
        }

        else if (reader_state == StressFactor)
        {
          if (directive == "end") reader_state = Main;

          /*
          else if (read_property("torque", handling.stress_factor.torque)) {}
          else if (read_property("braking", handling.stress_factor.braking)) {}
          else if (read_property("cornering", handling.stress_factor.cornering)) {}
          else if (read_property("antislide", handling.stress_factor.antislide)) {}
          */
        }

        /*
        else if (reader_state == CollisionShape)
        {
          if (directive == "circle")
          {
            float x, y, radius, bounciness;
            if (ArrayStream(remainder) >> x >> y >> radius >> bounciness)
            {
              collision_shapes::Circle circle;
              circle.center = { x, y };
              circle.radius = radius;
              circle.height = 1;              

              car_def.collision_shape.sub_shapes.push_back({ circle, bounciness });
            }
          }

          else if (directive == "polygon")
          {            
            float bounciness;
            if (ArrayStream(remainder) >> bounciness)
            {
              collision_shapes::Polygon polygon;
              polygon.height = 1;
              polygon.num_points = 0;

              car_def.collision_shape.sub_shapes.push_back({ polygon, bounciness });
              reader_state = CollisionPolygon;
            }
          }
        }

        else if (reader_state == CollisionPolygon)
        {
          if (directive == "end") reader_state = CollisionShape;
          else if (directive == "point")
          {
            float x, y;
            if (ArrayStream(remainder) >> x >> y)
            {
              auto& sub_shape = car_def.collision_shape.sub_shapes.back();
              auto& polygon = boost::get<collision_shapes::Polygon&>(sub_shape.data);

              collision_shapes::Polygon::Point point;
              point.position = { x, y };
              polygon.points[polygon.num_points] = point;
              ++polygon.num_points;              
            }
          }
        }
        */

        else if (directive == "end")
        {
          reader_state = End;        
        }

        else if (directive == "image" || directive == "rotimage")
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

        else if (directive == "collisionshape")
        {
          reader_state = CollisionShape;
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

        else if (directive == "downforceeffect")
        {
          reader_state = DownforceEffect;
        }

        else if (directive == "stressfactor")
        {
          reader_state = StressFactor;
        }

        /*
        else if (read_property_1000("torque", handling.torque)) {}
        else if (read_property("extratorque", handling.extra_torque)) {}
        else if (read_property("maxenginerevs", handling.max_engine_revs)) {}

        else if (read_property_1000("braking", handling.braking)) {}       

        else if (read_property("steering", handling.steering)) {}
        else if (read_property("minturningradius", handling.min_turning_radius)) {}

        else if (read_property_1000("cornering", handling.cornering)) {}
        else if (read_property_1000("antislide", handling.antislide)) {}     

        else if (read_property_1000("tractionlimit", handling.traction_limit)) {}

        else if (read_property("downforcecoefficient", handling.downforce_coefficient)) {}  

        else if (read_property("dragcoefficient", handling.drag_coefficient)) {}
        else if (read_property_1000("rollingresistance", handling.rolling_resistance)) {}

        else if (read_property("inputmoderation", handling.input_moderation)) {}

        else if (read_property("mass", handling.mass)) {}

        else if (read_property("loadtransfer", handling.load_transfer)) {}

        else if (read_property("reversegear", handling.reverse_gear_ratio)) {}
        */
      }

      return line_it;
    }

    template <typename LineIt, typename OutIt>
    static std::size_t load_cars_from_line_data(LineIt begin, LineIt end, boost::string_ref working_directory, 
                                                PatternStore& pattern_loader, OutIt out)
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
                                      pattern_store_, std::back_inserter(car_definitions_));
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