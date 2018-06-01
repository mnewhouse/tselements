/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "entity.hpp"
#include "control_point_manager.hpp"
#include "terrain_map.hpp"

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

    class PhysicsSpace
    {
    public:
      explicit PhysicsSpace(Vector2d size);

      void update(std::uint32_t frame_duration);

      void add_entity(Entity* entity);

      void add_static_circle();
      void add_static_polygon();
      
      struct UserData
      {
        Vector2d size;
      };

    private:
      struct Deleter
      {
        void operator()(void*) const;
      };      
      
      std::unique_ptr<void, Deleter> physics_space_;
      std::unique_ptr<UserData> user_data_;
    };

    // The World class manages all objects related to the physical state of the game.
    // Track terrains, cars, projectiles, dynamic scenery objects, and maybe more?
    // Updating the world state also happens through this class.
    class World
    {
    public:
      World(resources::Track track, TerrainMap terrain_map);

      Vector2d world_size() const;

      void update(std::uint32_t frame_duration, world::EventInterface& event_interface);

      Car* create_car(const CarDefinition& car_definition, std::uint8_t car_id, std::uint16_t start_pos);

      const Car* find_car(std::uint8_t car_id) const;
      Car* find_car(std::uint8_t car_id);

      using car_range = boost::iterator_range<boost::indirect_iterator<Car* const*, const Car>>;
      car_range cars() const;
      
      const resources::Track& track() const noexcept;

      resources::TerrainDefinition terrain_at(Vector2i position) const;
      resources::TerrainDefinition terrain_at(Vector2i position, std::int32_t level) const;

      resources::TerrainDefinition terrain_at(Vector2d position) const;
      resources::TerrainDefinition terrain_at(Vector2d position, std::int32_t level) const;

    private:
      Vector2<double> accomodate_position(Vector2<double> position) const;
      double accomodate_z_position(double z_position) const;

      std::vector<Car*> cars_;
      std::vector<std::unique_ptr<Entity>> entity_map_;

      resources::Track track_;
      TerrainMap terrain_map_;
      ControlPointManager control_point_manager_;

      PhysicsSpace physics_space_;
    };
  }
}
