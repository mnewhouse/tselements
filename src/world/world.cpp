/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "world.hpp"
#include "car.hpp"
#include "world_limits.hpp"
#include "collisions.hpp"
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

#include <vector>
#include <cmath>

namespace ts
{
  namespace world
  {
    namespace detail
    {
      static auto wall_test(const resources::TerrainLibrary& terrain_library)
      {
        return [&](resources::TerrainId terrain_id, std::uint32_t level)
        {
          return terrain_library.terrain(terrain_id, static_cast<std::uint8_t>(level)).is_wall;
        };
      }

      struct CollisionInfo
      {
        bool collided = false;
        bool stuck = false;
        bool rotate = true;
        Vector2i point;
        Vector2<double> valid_position;
        Vector2<double> collision_position;

        explicit operator bool() const
        {
          return collided;
        }
      };

      void resolve_collision(Entity* entity, Vector2<double> old_position, 
                             Rotation<double> old_rotation, Rotation<double> new_rotation,
                             const CollisionInfo& collision, const resources::CollisionMaskFrame& scenery,
                             const resources::TerrainDefinition& wall_terrain)
      {
        auto position_offset = collision.collision_position - old_position;
        auto local_point = vector2_cast<double>(collision.point) - collision.collision_position;
        auto rotation_delta = new_rotation - old_rotation;

        position_offset += local_point + transform_point(local_point, rotation_delta);

        auto collision_info = examine_scenery_collision(scenery, collision.point,
                                                        position_offset);

        resolve_scenery_collision(collision_info, *entity, rotation_delta,
                                  wall_terrain.bounciness * entity->bounciness());
      }

      auto find_collision(Vector2<double> old_position, Vector2<double> target_position,
                          const resources::CollisionMaskFrame& collision_frame,
                          const resources::CollisionMaskFrame& scenery_frame)
      {
        CollisionInfo result;
        result.valid_position = old_position;

        utility::LinePlottingRange position_range(old_position, target_position);
        for (auto position : position_range)
        {
          if (auto collision = test_scenery_collision(collision_frame, scenery_frame,
                                                      vector2_cast<std::int32_t>(position)))
          {
            result.collided = true;
            result.stuck = (vector2_cast<std::int32_t>(old_position) == vector2_cast<std::int32_t>(position));
            result.rotate = !result.stuck;
            result.point = collision.point;
            result.collision_position = position;            
            return result;
          }

          result.valid_position = position;
        }

        return result;
      }

      struct CollisionAvoidance
      {
        CollisionInfo collision;
        Vector2<double> offset;
      };

      CollisionAvoidance find_collision_avoidance(Vector2<double> old_position, Vector2<double> target_position,
                                                  const resources::CollisionMaskFrame& collision_frame,
                                                  const resources::CollisionMaskFrame& scenery_frame)
                                        
      {
        // If moving the entity by one pixel in the new direction results in a collision, then do something about it.
        auto position_offset = target_position - old_position;

        auto dx = position_offset.x >= 0.0 ? 1.0 : -1.0;
        auto dy = position_offset.y >= 0.0 ? 1.0 : -1.0;

        CollisionAvoidance avoidance_attempts[4];
        avoidance_attempts[0].offset = { dx, 0.0 };
        avoidance_attempts[1].offset = { 0.0, dy };
        avoidance_attempts[2].offset = { -dx, 0.0 };
        avoidance_attempts[3].offset = { 0.0, -dy };

        if (std::abs(position_offset.x) < std::abs(position_offset.y))
        {
          // y axis will take priority over x axis.
          std::swap(avoidance_attempts[0], avoidance_attempts[1]);
          std::swap(avoidance_attempts[2], avoidance_attempts[3]);
        }

        CollisionInfo collision_info[4] = {};
        for (auto& attempt : avoidance_attempts)
        {
          auto offset = attempt.offset;
          attempt.collision = find_collision(old_position + offset, target_position + offset,
                                             collision_frame, scenery_frame);

          if (!attempt.collision)
          {
            return attempt;
          }
        }

        auto distance = [&](const auto& attempt)
        {
          auto collision_pos = attempt.collision.collision_position;
          auto offset = collision_pos - (old_position + attempt.offset);
          return std::max(std::abs(offset.x), std::abs(offset.y));
        };

        auto stuck = [&](const auto& attempt)
        {
          return attempt.collision.stuck;
        };

        auto end = std::remove_if(std::begin(avoidance_attempts), std::end(avoidance_attempts), stuck);
        if (end != std::begin(avoidance_attempts))
        {
          auto attempt_it = std::begin(avoidance_attempts);
          auto best_attempt = attempt_it;
          auto max_distance = distance(*attempt_it);

          for (; attempt_it != end; ++attempt_it)
          {
            auto d = distance(*attempt_it);
            if (d < max_distance)
            {
              best_attempt = attempt_it;
              max_distance = d;
            }
          }

          return *best_attempt;
        }

        return CollisionAvoidance();
      }

      CollisionAvoidance find_collision_avoidance(Vector2<double> old_position, Vector2<double> target_position,
                                                  const resources::CollisionMaskFrame& old_collision_frame,
                                                  const resources::CollisionMaskFrame& new_collision_frame,
                                                  const resources::CollisionMaskFrame& scenery_frame)
      {
        auto avoidance = detail::find_collision_avoidance(old_position, target_position,
                                                          new_collision_frame, scenery_frame);

        if (avoidance.collision && old_collision_frame.row_begin(0) != new_collision_frame.row_begin(0))
        {
          avoidance.collision.rotate = true;
          auto unrotated = detail::find_collision_avoidance(old_position, target_position,
                                                            old_collision_frame, scenery_frame);
          if (!unrotated.collision)
          {
            avoidance = unrotated;
            avoidance.collision.rotate = false;            
          }
        }

        return avoidance;
      }
    }

    World::World(resources::Track track, resources::Pattern pattern)
      : track_(std::move(track)), 
        pattern_(std::move(pattern)),
        collision_mask_(pattern_, track_.height_level_count(), detail::wall_test(track_.terrain_library())),
        control_point_manager_(track_.control_points().data(), track_.control_points().size())
    {
      cars_.reserve(limits::max_car_count);
      entity_map_.resize(limits::max_car_count);
      previously_collided_.resize(limits::max_car_count, false);
    }

    Car* World::create_car(const CarDefinition& car_definition, std::uint8_t car_id)
    {
      auto entity_id = car_id_to_entity_id(car_id);

      if (cars_.size() >= limits::max_car_count || entity_map_[entity_id] != nullptr) 
        return nullptr;

      auto car = std::make_unique<Car>(car_definition, entity_id);
      auto car_ptr = car.get();

      cars_.push_back(car_ptr);
      entity_map_[entity_id] = std::move(car);

      auto track_size = track().size();

      auto position = make_vector2(0.0, 0.0);
      auto rotation = degrees(0.0);
      auto z_position = 0.0;

      const auto& start_points = track_.start_points();
      if (!start_points.empty())
      {
        auto point = start_points.front();
        position = vector2_cast<double>(point.position);
        rotation = degrees(static_cast<double>(point.rotation));
        z_position = static_cast<double>(point.level);
      }

      car_ptr->set_position(position);
      car_ptr->set_rotation(rotation);
      car_ptr->set_z_position(z_position);

      return car_ptr;
    }

    void World::update(world::EventInterface& event_interface, std::uint32_t frame_duration)
    {
      double fd = frame_duration * 0.001;

      for (Car* car : cars_)
      {
        auto entity_id = car->entity_id();
        auto z_level = car->z_level();

        auto old_position = car->position();
        auto old_rotation = car->rotation();
        car->update(terrain_at(old_position, z_level), fd);

        auto new_position = accomodate_position(car->position() + fd * car->velocity());
        auto new_rotation = radians(car->rotation().radians() + fd * car->rotating_speed());

        auto scenery_frame = collision_mask_.frame(z_level);
        auto collision_frame = car->collision_mask()->rotation_frame(new_rotation);
        auto old_collision_frame = car->collision_mask()->rotation_frame(old_rotation);

        // Move the car through all pixels in a linear fashion from old to new position.
        // Test for collisions with the scenery at every step of the way.
        auto collision = detail::find_collision(old_position, new_position, collision_frame, scenery_frame);
        if (collision)
        {
          if (previously_collided_[entity_id])
          {
            // First, attempt to avoid a collision after applying the new rotation
            auto avoidance = detail::find_collision_avoidance(old_position, new_position, 
                                                              old_collision_frame, collision_frame, scenery_frame);

            collision = avoidance.collision;
            new_position += avoidance.offset;
          }
        }

        if (collision /* && scenery collision */)
        {
          detail::resolve_collision(car, old_position, old_rotation, new_rotation, collision, scenery_frame,
                                    terrain_at(collision.point, z_level));

          new_position = collision.valid_position;
          if (!collision.rotate) new_rotation = old_rotation;
        }

        car->set_position(new_position);
        car->set_rotation(new_rotation);

        previously_collided_[entity_id] = collision.collided;

        auto cp_hit_callback = [&](const ControlPoint& point, double time_point)
        {
          auto frame_offset = static_cast<std::uint32_t>(frame_duration * time_point);

          event_interface.on_control_point_hit(car, point, frame_offset);
        };

        control_point_manager_.test_control_point_intersections(old_position, car->position(), cp_hit_callback);
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

    Vector2<double> World::accomodate_position(Vector2<double> position) const
    {
      if (position.x < 0.0) position.x = 0.0;
      if (position.y < 0.0) position.y = 0.0;

      auto bounds = world_size() - make_vector2(0.25, 0.25);
      if (position.x > bounds.x) position.x = bounds.x;
      if (position.y > bounds.y) position.y = bounds.y;

      return position;
    }

    const resources::Track& World::track() const noexcept
    {
      return track_;
    }

    const resources::TerrainDefinition& World::terrain_at(Vector2<double> position, std::uint32_t level) const
    {
      return terrain_at(vector2_cast<std::int32_t>(position), level);
    }

    const resources::TerrainDefinition& World::terrain_at(Vector2i position, std::uint32_t level) const
    {
      auto terrain_id = pattern_(position.x, position.y);
      const auto& terrain_library = track().terrain_library();

      return terrain_library.terrain(terrain_id, level);
    }
  }
}