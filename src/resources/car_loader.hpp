/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CAR_LOADER_HPP_891284
#define CAR_LOADER_HPP_891284

#include "car_definition.hpp"
#include "pattern_loader.hpp"

#include <boost/utility/string_ref.hpp>

#include <vector>
#include <istream>

namespace ts
{
  namespace resources
  {
    class CarLoader
    {
    public:
      // These functions can be used to load all cars in a car file or stream. 
      // The car definitions are added to the CarLoader's internal state, which can later
      // be retrieved by using get_result.
      std::size_t load_cars_from_file(const std::string& file_name);
      std::size_t load_cars_from_stream(std::istream& stream, boost::string_ref working_directory);
      
      // This function can be used to examine our vector of car definitions,
      // useful if move semantics are not desirable.
      const std::vector<CarDefinition>& car_definitions() const;

      // This function can be used to efficiently get the car definitions that we just loaded.
      // Our car definitions are transferred to the caller, leaving us with nothing.
      std::vector<CarDefinition> get_result();

    private:
      std::vector<CarDefinition> car_definitions_;
      PatternLoader pattern_loader_;
    };
  }
}

#endif