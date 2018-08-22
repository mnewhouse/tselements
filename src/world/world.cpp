/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/


#include "world.hpp"
#include "car.hpp"
#include "world_limits.hpp"
#include "control_point_manager_detail.hpp"
#include "entity_id_conversion.hpp"
#include "world_event_interface.hpp"

#include "resources/terrain_library.hpp"
#include "resources/car_definition.hpp"
#include "resources/terrain_definition.hpp"
#include "resources/collision_mask_detail.hpp"

#include "utility/random.hpp"
#include "utility/line_plotter.hpp"
#include "utility/debug_log.hpp"
#include "utility/math_utilities.hpp"

#include <boost/function_output_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/optional.hpp>

#include <chipmunk/chipmunk.h>

#include <vector>
#include <iostream>
#include <cmath>

namespace ts
{
  namespace world
  {
    static void update_body_position(cpBody* body, cpFloat dt)
    {
      cpBodyUpdatePosition(body, dt);

      auto pos = cpBodyGetPosition(body);
      auto space = cpBodyGetSpace(body);
      
      auto user_data = static_cast<const PhysicsSpace::UserData*>(cpSpaceGetUserData(space));      

      pos.x = cpfclamp(pos.x, 0.0, user_data->size.x);
      pos.y = cpfclamp(pos.y, 0.0, user_data->size.y);
      cpBodySetPosition(body, pos);
    }

    PhysicsSpace::PhysicsSpace(Vector2d size)
      : physics_space_(static_cast<void*>(cpSpaceNew())),
      user_data_(std::make_unique<UserData>())
    {     
      auto space = static_cast<cpSpace*>(physics_space_.get());
      cpSpaceSetUserData(space, user_data_.get());      

      user_data_->size = size;
    }

    void PhysicsSpace::update(std::uint32_t frame_duration)
    {
      auto fd = frame_duration * 0.001;

      cpSpaceStep(static_cast<cpSpace*>(physics_space_.get()), fd);
    }

    void PhysicsSpace::add_entity(Entity* entity)
    {
      auto body = static_cast<cpBody*>(entity->physics_body_.get());

      cpSpaceAddBody(static_cast<cpSpace*>(physics_space_.get()), body);

      cpBodySetPositionUpdateFunc(body, update_body_position);
    }

    void PhysicsSpace::Deleter::operator()(void* space) const
    {
      cpSpaceFree(static_cast<cpSpace*>(space));
    }

    World::World(resources::Track track, TerrainMap terrain_map)
      : track_(std::move(track)),
      terrain_map_(std::move(terrain_map)),
      control_point_manager_(track_.control_points().data(), track_.control_points().size()),
      physics_space_(vector2_cast<double>(track_.size()))
    {
      entity_map_.resize(limits::max_car_count);
      cars_.reserve(limits::max_car_count);
    }

    Car* World::create_car(const CarDefinition& car_definition, std::uint8_t car_id, std::uint16_t start_pos)
    {
      auto entity_id = car_id_to_entity_id(car_id);

      const auto& start_points = track_.start_points();
      if (cars_.size() >= limits::max_car_count || entity_map_[entity_id] != nullptr || start_pos >= start_points.size())
        return nullptr;

      auto position = make_vector2(0.0, 0.0);
      auto rotation = degrees(0.0);
      auto z_position = 0.0;

      auto point = start_points[start_pos];
      position = vector2_cast<double>(point.position);
      rotation = degrees(static_cast<double>(point.rotation));
      z_position = static_cast<double>(point.level);

      auto car = std::make_unique<Car>(car_definition, entity_id);
      auto car_ptr = car.get();

      cars_.push_back(car_ptr);
      entity_map_[entity_id] = std::move(car);

      physics_space_.add_entity(car_ptr);

      car_ptr->set_position(position);
      car_ptr->set_rotation(rotation);
      car_ptr->set_z_position(z_position);

      return car_ptr;
    }

    void World::update(std::uint32_t frame_duration, world::EventInterface& event_interface)
    {
      double fd = frame_duration * 0.001;

      auto max_corner = vector2_cast<std::int32_t>(world_size()) - make_vector2(1, 1);

      const auto& terrain_lib = track_.terrain_library();

      entity_states_.clear();      
      for (auto* car : cars_)
      {
        entity_states_.push_back({ car, car->position() });

        car->update(*this, fd);
      }

      physics_space_.update(frame_duration);

      for (auto& es : entity_states_)
      {
        auto cp_hit_callback = [&](const ControlPoint& point, double time_point)
        {
          auto frame_offset = static_cast<std::uint32_t>(frame_duration * time_point);

          event_interface.on_control_point_hit(es.entity, point, frame_offset);
        };

        control_point_manager_.test_control_point_intersections(es.old_position, es.entity->position(), cp_hit_callback);
      }
    }

    Car* World::find_car(std::uint8_t car_id)
    {
      auto entity_id = car_id_to_entity_id(car_id);
      return static_cast<Car*>(entity_map_[entity_id].get());
    }

    const Car* World::find_car(std::uint8_t car_id) const
    {
      auto entity_id = car_id_to_entity_id(car_id);
      return static_cast<const Car*>(entity_map_[entity_id].get());
    }

    World::car_range World::cars() const
    {
      return car_range(cars_.data(), cars_.data() + cars_.size());
    }

    Vector2<double> World::world_size() const
    {
      return vector2_cast<double>(track().size());
    }

    const resources::Track& World::track() const noexcept
    {
      return track_;
    }

    resources::TerrainDefinition World::terrain_at(Vector2i position) const
    {      
      return terrain_at(position, 0);
    }

    resources::TerrainDefinition World::terrain_at(Vector2i position, std::int32_t level) const
    {
      return terrain_map_.terrain_at(position, level, track_.terrain_library());
    }

    resources::TerrainDefinition World::terrain_at(Vector2d position) const
    {
      return terrain_at(position, 0);
    }

    resources::TerrainDefinition World::terrain_at(Vector2d position, std::int32_t level) const
    {
      return terrain_at(vector2_cast<std::int32_t>(position), level);
    }
  }
}