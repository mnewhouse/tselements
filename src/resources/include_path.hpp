/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef INCLUDE_PATH_HPP_446932
#define INCLUDE_PATH_HPP_446932

#include <string>
#include <initializer_list>

#include <boost/utility/string_ref.hpp>
#include <boost/filesystem/path.hpp>

namespace ts
{
  namespace resources
  {
    // These are utility functions that take a file name and a list of search directories, 
    // iterating over these directories until the specified file name is found in one of them.
    // find_include_directory returns only the directory, find_include_path returns the full path.
    // If the file is not found, an empty-state object is returned.
    // Paths are expected to be UTF-8 encoded.

    boost::string_ref find_include_directory(boost::string_ref file_name, std::initializer_list<boost::string_ref> search_paths);
    boost::filesystem::path find_include_path(boost::string_ref file_name, std::initializer_list<boost::string_ref> search_paths);
  }
}

#endif

