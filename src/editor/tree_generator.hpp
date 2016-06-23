/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef TREE_GENERATOR_HPP_26829834523
#define TREE_GENERATOR_HPP_26829834523

#include "model_3d.hpp"

#include <utility/color.hpp>

#include <cstdint>
#include <vector>

namespace ts
{
  namespace editor
  {
    namespace track
    {
      struct TreeBranchProperties
      {
        float initial_weight = 4.0f;
        float branch_weight = 0.8f;
        float weight_threshold = 0.9f;
        float num_sides = 5;
        float segment_length = 1.0f;
        Colorb color;
      };

      struct LeafProperties
      {
        float density;
        Colorb color;
        Colorb color_variance;
      };

      namespace detail
      {
        inline void generate_tree_branch(std::uint32_t recursion_depth,
                                         Vector3f base_position,
                                         Vector3f base_direction,
                                         const TreeBranchProperties& branch_properties,
                                         const LeafProperties& leaf_properties,
                                         resources_3d::Model& model)
        {
          // A tree branch starts at base_position and goes towards base_direction.
          // The branch curves randomly, with a likelihood of sloping downward
          // because the smaller width near the end makes it less sturdy.
          // Branches should be distributed evenly across the whole tree, so that
          // the probability of collisions is low.
          // Features
          // * Branch weight
          // * Curvature
          // * Splitting
          // * Recursive branches
          // * Leaves
          // To start, create a branch segment that goes from base_position towards
          // base_direction, where num_sides determines the number of faces, and therefore
          // the level of detail of the branch. Then, keep adding segments until the
          // branch weight drops below a certain threshold, adjusting the segment properties
          // with each one.
          std::uint32_t vertex_index = model.vertices.size();

          struct SegmentInfo
          {
            Vector3f start_position;
            Vector3f end_position;
          };

          auto weight = branch_properties.initial_weight;
          while (weight >= branch_properties.weight_threshold)
          {

          }
        }

        inline void generate_tree_trunk(std::uint32_t recursion_depth,
                                        Vector3f base_direction,
                                        const TreeBranchProperties& trunk_properties,
                                        const LeafProperties& leaf_properties,
                                        resources_3d::Model& tree_model)
        {
          generate_tree_branch(recursion_depth, Vector3f(0.0f, 0.0f, 0.0f), base_direction,
                               trunk_properties, leaf_properties,
                               tree_model);
        }
      }
      
      inline auto generate_tree_model(std::uint32_t recursion_levels,
                                      Vector3f base_direction,
                                      const TreeBranchProperties& trunk_properties,
                                      const LeafProperties& leaf_properties)

      {
        resources_3d::Model tree_model;

        detail::generate_tree_trunk(recursion_levels, base_direction, trunk_properties,
                                    leaf_properties, tree_model);

        return tree_model;
      }
    }
  }
}

#endif