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

      enum ReaderState
      {
        Main, Handling, CollisionShape, CollisionPolygon, End
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

        if (directive == "end")
        {
          if (reader_state == Handling)
          {
            reader_state = Main;
          }
          else
          {
            reader_state = End;
          }
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

        else if (directive == "handling")
        {
          reader_state = Handling;
        }

        else if (read_property("mass", car_def.mass)) {}
        else if (read_property("bounciness", car_def.bounciness)) {}
        else if (read_property("momentofinertia", car_def.moment_of_inertia)) {}
        else if (directive == "centerofmass")
        {
          Vector2d com;
          if (ArrayStream(remainder) >> com.x >> com.y)
          {
            car_def.center_of_mass = com;
          }
          
          else insufficient_parameters(car_name, directive);
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

        else if (reader_state == Handling)
        {
          auto& h = car_def.handling;

          if (read_property("acceleration", h.max_acceleration_force)) {}
          else if (read_property("braking", h.max_braking_force)) {}
          else if (read_property("reversegearratio", h.reverse_gear_ratio)) {}
          else if (read_property("gearshiftduration", h.gear_shift_duration)) {}
          else if (read_property("drag", h.drag_coefficient)) {}
          else if (read_property("downforce", h.downforce_coefficient)) {}
          else if (read_property("rollingdrag", h.rolling_drag_coefficient)) {}
          else if (read_property("tractionlimit", h.traction_limit)) {}
          else if (read_property("loadtransfer", h.load_transfer)) {}
          else if (read_property("brakebalance", h.brake_balance)) {}
          else if (read_property("downforcebalance", h.downforce_balance)) {}
          else if (read_property("steeringbalance", h.steering_balance)) {}
          else if (read_property("cornering", h.cornering)) {}
          else if (read_property("maxsteeringangle", h.max_steering_angle)) {}
          else if (read_property("nonslideangle", h.non_slide_angle)) {}
          else if (read_property("fullslideangle ", h.full_slide_angle)) {}
          else if (read_property("slidinggrip", h.sliding_grip)) {}
          else if (read_property("angulardamping", h.angular_damping)) {}

          else if (read_property("wheelbaselength", h.wheelbase_length)) {}
          else if (read_property("wheelbaseoffset", h.wheelbase_offset)) {}
          else if (read_property("numfrontwheels", h.num_front_wheels)) {}
          else if (read_property("numrearwheels", h.num_rear_wheels)) {}
          else if (read_property("frontaxlewidth", h.front_axle_width)) {}
          else if (read_property("rearaxlewidth", h.rear_axle_width)) {}

          else if (directive == "frontdriven")
          {
            int d{};
            ArrayStream(remainder) >> d;
            h.front_driven = (d != 0);
          }

          else if (directive == "reardriven")
          {
            int d{};
            ArrayStream(remainder) >> d;
            h.rear_driven = (d != 0);
          }

          else if (directive == "gears")
          {
            h.gear_ratios.clear();

            ArrayStream stream(remainder);
            double r;
            while (stream >> r)
            {
              h.gear_ratios.push_back(r);
            }

            if (h.gear_ratios.empty())
            {
              h.gear_ratios.push_back(1.0f);
            }
          }            
        }

        /*

        double max_engine_revs = 300.0;

        // Gear ratios
        boost::container::small_vector<double, 8> gear_ratios = { 2.5, 2.0, 1.6, 1.3, 1.1 };
        double reverse_gear_ratio = 2.3;

        // Cornering: how much of any wheel's traction limit is available for cornering.
        double cornering = 1.0;

        // Maximum steering angle. Increase this to increase the minimum turning radius,
        // and also to make it less likely the car spins out of control.
        double max_steering_angle = 30.0;

        // Slip angles in degrees at which the car is not/fully sliding.
        double non_slide_angle = 4.5;
        double full_slide_angle = 8.5;

        // For a fully sliding wheel, traction limit is multiplied by sliding_grip.
        double sliding_grip = 0.87;

        // The amount the angular velocity is decreased by every second.
        // 1.0 means the angular velocity is reduced by 2% every 20ms.
        double angular_damping = 1.3;

        // Wheel positions have an effect on the car's willingness to rotate.
        // Longer wheelbase means more torque is applied to the car, making it spin more easily.
        double wheelbase_length = 18.0;
        double wheelbase_offset = 0.0;
        double num_front_wheels = 2;
        double num_rear_wheels = 2;
        double front_axle_width = 6.0;
        double rear_axle_width = 6.0;

        // Whether the car is driven at the front, rear or both.
        bool front_driven = false;
        bool rear_driven = true;
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