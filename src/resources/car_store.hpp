/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CAR_STORE_HPP_77112905
#define CAR_STORE_HPP_77112905

#include "car_description.hpp"
#include "car_definition.hpp"
#include "resource_library_interface.hpp"

#include <set>
#include <string>
#include <vector>

#include <boost/range/iterator_range.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/utility/string_ref.hpp>

namespace ts
{
  namespace resources
  {
    namespace detail
    {
      struct CarDefinitionComparator
      {
        using is_transparent = std::true_type;

        bool operator()(const CarDefinition& a, const CarDefinition& b) const;
        bool operator()(const CarDefinition& car_def, const CarDescriptionRef& car_desc) const;
        bool operator()(const CarDescriptionRef& car_desc, const CarDefinition& car_def) const;
        bool operator()(const CarDefinition& car_def, boost::string_ref car_name) const;
        bool operator()(boost::string_ref car_name, const CarDefinition& car_def) const;
      };
    }

    class CarLoader;

    // The CarStore provides an interface to load all car models
    // from a specified directory, and access them at a later time.
    class CarStore
    {
    public:
      void load_car_directory(const std::string& directory, const std::string& extension = ".car");
      void load_car_file(const std::string& file);

      using car_definition_set = std::set<CarDefinition, detail::CarDefinitionComparator>;
      using car_definition_interface = ResourceLibraryInterface<car_definition_set, SetSearchPolicy>;
      using car_definition_range = boost::iterator_range<boost::indirect_iterator<const CarDefinition* const*>>;

      car_definition_interface car_definitions() const;
      car_definition_range sequential_car_definitions() const;

    private:
      void store_car_definitions(const CarLoader& car_loader);

      car_definition_set loaded_cars_;
      std::vector<const CarDefinition*> load_order_;
    };
  }
}

#endif