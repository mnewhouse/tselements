/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef WORLD_HPP_5590592
#define WORLD_HPP_5590592

#include "entity.hpp"
#include "control_point_manager.hpp"

#include "resources/track.hpp"
#include "resources/pattern.hpp"
#include "resources/collision_mask.hpp"

#include "utility/vector2.hpp"

#include <boost/range/iterator_range.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/iterator_adaptors.hpp>
#include <boost/container/flat_set.hpp>

#include <cstdint>
#include <vector>
#include <memory>

namespace ts
{
  namespace resources
  {
    struct CarDefinition;
    struct TerrainDefinition;
    class Track;
    class Pattern;
  }

  namespace controls
  {
    class Controllable;
  }

  namespace world
  {
    class Car;
    class Entity;
    using resources::CarDefinition;
    using controls::Controllable;

    struct EventInterface;

    namespace detail
    {
      struct EntityCollisionFrame
      {
        Entity* entity;
        Vector2i position;
        resources::CollisionMaskFrame collision_frame;
      };

      struct EntityUpdateState;

      struct EntityUpdateBoundingBox
      {
        EntityUpdateState* update_state;
        IntRect bounding_box;
      };

      struct EntityUpdateState
      {
        Entity* entity;
        Rotation<double> old_rotation;
        Rotation<double> new_rotation;
        Vector2<double> old_position;
        Vector2<double> new_position;

        std::uint32_t z_level;
        Vector2i current_position;

        resources::CollisionMaskFrame old_frame;
        resources::CollisionMaskFrame new_frame;
      };
    }

    // The World class manages all objects related to the physical state of the game.
    // Track terrains, cars, projectiles, dynamic scenery objects, and maybe more?
    // Updating the world state also happens through this class.
    class World
    {
    public:
      World(resources::Track track, resources::Pattern pattern);

      Vector2<double> world_size() const;

      void update(world::EventInterface& event_interface, std::uint32_t frame_duration);

      Car* create_car(const CarDefinition& car_definition, std::uint8_t car_id, std::uint16_t start_pos);

      const Car* find_car(std::uint8_t car_id) const;
      Car* find_car(std::uint8_t car_id);

      using car_range = boost::iterator_range<boost::indirect_iterator<Car* const*, const Car>>;
      car_range cars() const;
      
      const resources::Track& track() const noexcept;

      const resources::TerrainDefinition& terrain_at(Vector2<double> position, std::uint32_t level = 0) const;
      const resources::TerrainDefinition& terrain_at(Vector2i position, std::uint32_t level = 0) const;

    private:
      Vector2<double> accomodate_position(Vector2<double> position) const;

      std::vector<Car*> cars_;
      std::vector<std::unique_ptr<Entity>> entity_map_;

      resources::Track track_;
      resources::Pattern pattern_;

      resources::CollisionMask collision_mask_;
      ControlPointManager control_point_manager_;

      std::vector<bool> previously_collided_;
      std::vector<detail::EntityUpdateState> entity_update_state_;
      std::vector<detail::EntityUpdateBoundingBox> bounding_box_cache_;
    };
  }
}

#endif