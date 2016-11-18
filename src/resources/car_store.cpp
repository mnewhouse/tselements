/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"

#include "car_store.hpp"
#include "car_loader.hpp"

#include "utility/string_utilities.hpp"
#include "utility/debug_log.hpp"

#include <boost/filesystem.hpp>

#include <tuple>

namespace bfs = boost::filesystem;

namespace ts
{
  namespace resources
  {
    namespace detail
    {
      bool CarDefinitionComparator::operator()(const CarDefinition& a, const CarDefinition& b) const
      {
        // Make sure to compare in a case-insensitive manner, *then* compare by hash
        return std::tie(caseless_string(a.car_name), a.car_hash) < std::tie(b.car_name, b.car_hash);
      }

      bool CarDefinitionComparator::operator()(const CarDefinition& car_def, const CarDescriptionRef& car_desc) const
      {
        return std::tie(caseless_string(car_def.car_name), car_def.car_hash) < std::tie(car_desc.name, car_desc.hash);
      }

      bool CarDefinitionComparator::operator()(const CarDescriptionRef& car_desc, const CarDefinition& car_def) const
      {
        return std::tie(caseless_string(car_desc.name), car_desc.hash) < std::tie(car_def.car_name, car_def.car_hash);
      }

      bool CarDefinitionComparator::operator()(const CarDefinition& car_def, boost::string_ref car_name) const
      {
        return caseless_string(car_def.car_name) < car_name;
      }

      bool CarDefinitionComparator::operator()(boost::string_ref car_name, const CarDefinition& car_def) const
      {
        return caseless_string(car_name) < car_def.car_name;
      }
    }

    void CarStore::load_car_directory(const std::string& directory, const std::string& extension)      
    {
      CarLoader car_loader;

      // Loop through all files in the given directory that have a matching extension
      for (bfs::directory_iterator dir_it(directory), end; dir_it != end; ++dir_it)
      {
        const auto& entry = *dir_it;
        if (bfs::is_regular_file(entry.path()) && bfs::extension(entry.path()) == extension)
        {
          // And load the file
          car_loader.load_cars_from_file(entry.path().string());     
        }
      }

      store_car_definitions(car_loader);
    }

    void CarStore::load_car_file(const std::string& car_file)
    {
      CarLoader car_loader;
      car_loader.load_cars_from_file(car_file);
      store_car_definitions(car_loader);
    }

    void CarStore::store_car_definitions(const CarLoader& car_loader)
    {
      for (const auto& car_def : car_loader.car_definitions())
      {
        auto result = loaded_cars_.insert(car_def);
        if (result.second)
        {
          DEBUG_AUXILIARY << "Loaded car '" << car_def.car_name << "' into car store." << debug::endl;
          load_order_.push_back(&*result.first);
        }
      }
    }
    
    CarStore::car_definition_interface CarStore::car_definitions() const
    {
      return car_definition_interface(&loaded_cars_);
    }

    CarStore::car_definition_range CarStore::sequential_car_definitions() const
    {
      return car_definition_range(load_order_.data(), load_order_.data() + load_order_.size());
    }
  }
}
