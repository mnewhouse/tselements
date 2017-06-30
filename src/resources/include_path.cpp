/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/


#include "include_path.hpp"

#include <boost/filesystem/convenience.hpp>

namespace ts
{
  namespace resources
  {
    namespace bfs = boost::filesystem;

    bfs::path find_include_path(boost::string_ref file_name,
                                std::initializer_list<boost::string_ref> search_paths)
    {
      bfs::path full_path;
      for (boost::string_ref path : search_paths)
      {
        full_path.assign(path.begin(), path.end());
        full_path.append(file_name.begin(), file_name.end());

        if (boost::filesystem::is_regular_file(full_path)) return full_path;
      }

      full_path.clear();
      return full_path;
    }

    boost::string_ref find_include_directory(boost::string_ref file_name,
                                             std::initializer_list<boost::string_ref> search_paths)
    {
      bfs::path full_path;
      for (boost::string_ref path : search_paths)
      {
        full_path.assign(path.begin(), path.end());
        full_path.append(file_name.begin(), file_name.end());

        if (boost::filesystem::is_regular_file(full_path)) return path;
      }

      return boost::string_ref();
    }
  }
}