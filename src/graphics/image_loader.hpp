/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef IMAGE_LOADER_HPP_58198491284
#define IMAGE_LOADER_HPP_58198491284

#include "image.hpp"

#include "utility/string_utilities.hpp"

#include <istream>
#include <string>
#include <map>
#include <new>

namespace ts
{
  namespace graphics
  {
    // The image loader class template stores a number of images/surfaces by file name,
    // so that it's easy to load them only once and cache them later.
    template <typename ImageType, typename LoadingFunc>
    class ImageLoader
    {
    public:
      explicit ImageLoader(LoadingFunc loading_func = {})
        : loading_func_(std::move(loading_func))
      {
      }

      template <typename StringType>
      ImageType& load_image(const StringType& file_name)
      {
        auto it = images_.find(file_name);
        if (it == images_.end())
        {
          using std::begin;
          using std::endl;

          std::string my_file_name(begin(file_name), end(file_name));
          it = images_.insert(std::make_pair(std::move(my_file_name), loading_func_(file_name))).first;
        }

        return it->second;
      }

    private:
      std::map<std::string, ImageType, std::less<>> images_;
      LoadingFunc loading_func_;
    };
  }
}

#endif