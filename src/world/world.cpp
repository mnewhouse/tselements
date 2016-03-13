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

#include <boost/function_output_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/optional.hpp>

#include <vector>
#include <iostream>
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

      struct CollisionTimePoint
      {
        std::int32_t step;
        std::int32_t total_steps;
      };

      inline bool operator<(const CollisionTimePoint& a, const CollisionTimePoint& b)
      {
        return a.step * b.total_steps < b.step * a.total_steps;
      }

      inline bool operator>(const CollisionTimePoint& a, const CollisionTimePoint& b)
      {
        return b < a;
      }

      struct CollisionInfo
      {
        const EntityUpdateState* subject_state = nullptr;
        const EntityUpdateState* object_state = nullptr;

        bool collided = false;
        bool stuck = false;
        bool rotate = true;
        Vector2i point;
        Vector2i collision_position;
        Vector2i valid_position;

        CollisionTimePoint time_point;

        explicit operator bool() const
        {
          return collided;
        }
      };

      auto resolve_collision(const CollisionInfo& collision)
      {
        auto subject = collision.subject_state->entity;
        auto object = collision.object_state->entity;

        auto collision_info = examine_entity_collision(collision.point, collision.collision_position,
                                                       collision.object_state->current_position, 
                                                       subject->velocity(), object->velocity(),
                                                       subject->bounciness() * object->bounciness());

        resolve_entity_collision(collision_info, *subject, *object);

        return collision_info;
      }

      auto resolve_collision(const CollisionInfo& collision, 
                             const resources::CollisionMaskFrame& scenery,
                             const resources::TerrainDefinition& wall_terrain)
      {
        const auto& update_state = *collision.subject_state;
        auto entity = update_state.entity;
        auto rotation_delta = update_state.new_rotation - update_state.old_rotation;
        auto bounce_factor = entity->bounciness() * wall_terrain.bounciness;

        auto collision_info = examine_scenery_collision(scenery, collision.point, entity->velocity(), 
                                                        entity->velocity(), bounce_factor);

        resolve_scenery_collision(collision_info, *entity, rotation_delta);
        return collision_info;
      }

      template <typename ObjectStateRange>
      CollisionInfo find_collision(const EntityUpdateState& update_state,
                                   const resources::CollisionMaskFrame& scenery_frame,
                                   const ObjectStateRange& object_states,
                                   Vector2i position_offset, bool rotate_subject)
      {
        auto old_position = vector2_cast<std::int32_t>(update_state.old_position) + position_offset;
        auto new_position = vector2_cast<std::int32_t>(update_state.new_position) + position_offset;

        const auto& collision_frame = rotate_subject ? update_state.new_frame : update_state.old_frame;

        CollisionInfo result;
        result.valid_position = old_position;        

        utility::LinePlottingRange position_range(old_position, new_position);
        result.time_point.step = 0;
        result.time_point.total_steps = position_range.step_count();

        for (auto position : position_range)
        {
          if (auto collision = test_scenery_collision(collision_frame, scenery_frame, position))
          {
            result.subject_state = &update_state;
            result.collided = true;
            result.stuck = old_position == position;
            result.rotate = !result.stuck;
            result.point = collision.point;
            result.collision_position = position;
            return result;
          }

          for (const auto& object_state : object_states)
          {
            if (auto collision = test_collision(collision_frame, object_state.old_frame,
                                                position, object_state.current_position))
            {
              result.collided = true;
              result.stuck = position == old_position;
              result.rotate = result.stuck && rotate_subject;

              result.collision_position = position;

              result.point = collision.point;
              result.subject_state = &update_state;
              result.object_state = &object_state;
              return result;
            }
          }

          result.valid_position = position;
          ++result.time_point.step;
        }

        return result;
      }

      struct CollisionAvoidance
      {
        CollisionInfo collision;
        Vector2<double> offset;
        bool rotate_subject;
      };

      template <typename ObjectStateRange>
      CollisionAvoidance find_collision_avoidance(const EntityUpdateState& update_state,
                                                  const resources::CollisionMaskFrame& scenery_frame,
                                                  const ObjectStateRange& object_states,
                                                  bool rotate_subject, const CollisionInfo& default_collision)                                    
      {
        // If moving the entity by one pixel in the new direction results in a collision, then do something about it.
        auto new_position = vector2_cast<std::int32_t>(update_state.new_position);
        auto old_position = vector2_cast<std::int32_t>(update_state.old_position);

        auto frame_offset = update_state.new_position - update_state.old_position;

        auto dx = frame_offset.x >= 0.0 ? 1.0 : -1.0;
        auto dy = frame_offset.y >= 0.0 ? 1.0 : -1.0;

        CollisionAvoidance avoidance_attempts[4] = {};
        avoidance_attempts[0].offset = { dx, 0.0 };
        avoidance_attempts[1].offset = { 0.0, dy };
        avoidance_attempts[2].offset = { -dx, 0.0 };
        avoidance_attempts[3].offset = { 0.0, -dy };

        if (std::abs(frame_offset.x) < std::abs(frame_offset.y))
        {
          // y axis will take priority over x axis.
          std::swap(avoidance_attempts[0], avoidance_attempts[1]);
          std::swap(avoidance_attempts[2], avoidance_attempts[3]);
        }

        for (auto& attempt : avoidance_attempts)
        {
          attempt.collision = find_collision(update_state, scenery_frame, object_states, attempt.offset, rotate_subject);
          attempt.rotate_subject = rotate_subject;

          if (!attempt.collision)
          {
            return attempt;
          }
        }

        auto range_end = std::remove_if(std::begin(avoidance_attempts), std::end(avoidance_attempts),
                                        [](const CollisionAvoidance& attempt)
        {
          return attempt.collision.stuck;
        });
        
        if (range_end == std::begin(avoidance_attempts))
        {
          CollisionAvoidance result;
          result.offset = {};
          result.collision = default_collision;
          result.rotate_subject = default_collision.rotate;
          return result;
        }

        else
        {
          return *std::min_element(avoidance_attempts, range_end,
                                   [](const CollisionAvoidance& a, const CollisionAvoidance& b)
          {
            return a.collision.time_point < b.collision.time_point;
          });
        }
      }

      template <typename ObjectStateRange>
      CollisionAvoidance find_collision_avoidance(const EntityUpdateState& update_state,
                                                  const resources::CollisionMaskFrame& scenery_frame,
                                                  const ObjectStateRange& object_states,
                                                  const CollisionInfo& default_collision)
      {
        auto avoidance = find_collision_avoidance(update_state, scenery_frame, object_states, true, default_collision);

        if (avoidance.collision && update_state.old_frame.row_begin(0) != update_state.new_frame.row_begin(0))
        {
          auto unrotated = find_collision_avoidance(update_state, scenery_frame, object_states, false, default_collision);

          if (!unrotated.collision || unrotated.collision.time_point < avoidance.collision.time_point)
          {
            avoidance = unrotated;
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
      entity_map_.resize(limits::max_car_count);
      cars_.reserve(limits::max_car_count);      
      previously_collided_.resize(limits::max_car_count);
      entity_update_state_.reserve(limits::max_car_count);
      bounding_box_cache_.reserve(limits::max_car_count * 2);
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

      auto track_size = track().size();

      car_ptr->set_position(position);
      car_ptr->set_rotation(rotation);
      car_ptr->set_z_position(z_position);

      return car_ptr;
    }

    void World::update(world::EventInterface& event_interface, std::uint32_t frame_duration)
    {
      double fd = frame_duration * 0.001;
      entity_update_state_.clear();
      for (auto* car : cars_)
      {
        auto old_position = car->position();
        auto old_rotation = car->rotation();

        car->update(terrain_at(old_position, car->z_level()), fd);

        auto new_position = accomodate_position(car->position() + fd * car->velocity());
        auto new_rotation = radians(car->rotation().radians() + fd * car->rotating_speed());

        entity_update_state_.push_back({
          car, old_rotation, new_rotation, old_position, new_position, car->z_level(),
          vector2_cast<std::int32_t>(old_position),
          car->collision_mask()->rotation_frame(old_rotation),
          car->collision_mask()->rotation_frame(new_rotation),
        });               
      }     

      bounding_box_cache_.clear();
      for (auto& update_state : entity_update_state_)
      {
        auto entity = update_state.entity;
        auto mask = entity->collision_mask();
        auto bb = mask->bounding_box();

        auto old_box = translate(bb, vector2_cast<std::int32_t>(update_state.old_position));
        auto new_box = translate(bb, vector2_cast<std::int32_t>(update_state.new_position));

        auto total_bb = combine(old_box, new_box);
        --total_bb.left;
        --total_bb.top;
        total_bb.width += 2;
        total_bb.height += 2;

        bounding_box_cache_.push_back({ &update_state, total_bb });
      }

      auto bb_it = bounding_box_cache_.begin();
      for (auto& update_state : entity_update_state_)
      {
        auto entity = update_state.entity;
        auto entity_id = entity->entity_id();
        auto z_level = update_state.z_level;

        auto old_position = entity->position();
        auto new_position = update_state.new_position;

        auto scenery_frame = collision_mask_.frame(z_level);
        const auto& new_collision_frame = update_state.new_frame;

        auto bounding_box = bb_it->bounding_box;

        auto bb_predicate = [=](const auto& entry)
        {
          return entry.update_state->z_level == z_level && intersects(entry.bounding_box, bounding_box);
        };

        std::size_t bb_cache_size = bounding_box_cache_.size();

        std::copy_if(bounding_box_cache_.begin(), bb_it, 
                     std::back_inserter(bounding_box_cache_), bb_predicate);
        ++bb_it;
        // TODO: Improve complexity, O(n^2) is not quite acceptable.
        std::copy_if(bb_it, bounding_box_cache_.begin() + bb_cache_size, 
                     std::back_inserter(bounding_box_cache_), bb_predicate);

        auto bb_transform = [](const auto& bb) -> detail::EntityUpdateState&
        {
          return *bb.update_state;
        };

        auto object_range = boost::make_iterator_range(
          boost::make_transform_iterator(bounding_box_cache_.begin() + bb_cache_size, bb_transform),
          boost::make_transform_iterator(bounding_box_cache_.end(), bb_transform)
          );

        auto collision = detail::find_collision(update_state, scenery_frame, object_range, {}, true);
        if (collision)
        {
          if (previously_collided_[entity_id])
          {
            auto avoidance = detail::find_collision_avoidance(update_state, scenery_frame, object_range, collision);
            update_state.old_position += avoidance.offset;
            update_state.new_position += avoidance.offset;

            collision = avoidance.collision;
            if (!collision && !avoidance.rotate_subject)
            {
              update_state.new_rotation = update_state.old_rotation;
              update_state.new_frame = update_state.old_frame;
            }
          }
        }

        if (collision)
        {
          auto position = vector2_cast<double>(collision.valid_position) + 0.5;

          {
            auto x_limits = std::minmax(old_position.x, new_position.x);
            auto y_limits = std::minmax(old_position.y, new_position.y);
            
            position.x = std::min(std::max(position.x, x_limits.first), x_limits.second);
            position.y = std::min(std::max(position.y, y_limits.first), y_limits.second);
          }

          entity->set_position(position);
          entity->set_rotation(collision.rotate ? update_state.new_rotation : update_state.old_rotation);
          update_state.current_position = collision.valid_position;

          if (collision.object_state)
          {
            auto collision_result = detail::resolve_collision(collision);

            event_interface.on_collision(entity, collision.object_state->entity, collision_result);            
          }

          else
          {
            auto scenery_frame = collision_mask_.frame(z_level);
            auto terrain = terrain_at(collision.point, z_level);
            auto collision_result = detail::resolve_collision(collision, scenery_frame, terrain);

            event_interface.on_collision(entity, collision_result);
          }
        }

        else
        {
          entity->set_position(update_state.new_position);
          entity->set_rotation(update_state.new_rotation);          
        }

        previously_collided_[entity_id] = collision.collided;
        bounding_box_cache_.resize(bb_cache_size);           

        auto cp_hit_callback = [&](const ControlPoint& point, double time_point)
        {
          auto frame_offset = static_cast<std::uint32_t>(frame_duration * time_point);

          event_interface.on_control_point_hit(entity, point, frame_offset);
        };

        update_state.current_position = vector2_cast<std::int32_t>(entity->position());
        control_point_manager_.test_control_point_intersections(old_position, entity->position(), cp_hit_callback);
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