/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef TRACK_EDITOR_TOOL_HPP_48591845
#define TRACK_EDITOR_TOOL_HPP_48591845

#include <boost/range/iterator_range.hpp>

namespace ts
{
  namespace editor
  {
    namespace track
    {
      enum class Tool
      {
        None,
        Path,
        Elevation,
        Terrain
      };
      
      namespace detail
      {
        const char* const path_tool_mode_names[] =
        {
          "Add Nodes",
          "Edit Nodes",
          "Stroke Segments",
        };
      }

      inline boost::iterator_range<const char* const*> mode_names_by_tool(Tool tool)
      {
        auto make_range = [](const auto& names)
        {
          return boost::make_iterator_range(std::begin(names),
                                            std::end(names));
        };

        switch (tool)
        {
        case Tool::Path:
          return make_range(detail::path_tool_mode_names);
        default:
          return{};
        }
      }
    }
  }
}

#endif