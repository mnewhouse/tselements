/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/


#include "world.hpp"
#include "car_detail.hpp"
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
#include "utility/math_utilities.hpp"

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

      struct UpdatePassInfo
      {
        CollisionInfo collision;
        double z_position;
      };

      template <typename ObjectStateRange>
      UpdatePassInfo entity_update_pass(const World& world_obj,
                                        const EntityUpdateState& update_state,
                                        const resources::CollisionMask& scenery,
                                        const ObjectStateRange& object_states,
                                        Vector2i position_offset, bool rotate_subject)
      {
        const auto& terrain_library = world_obj.track().terrain_library();

        auto world_size = vector2_cast<std::int32_t>(world_obj.world_size());        

        auto old_position = vector2_cast<std::int32_t>(update_state.old_position) + position_offset;
        auto new_position = vector2_cast<std::int32_t>(update_state.new_position) + position_offset;

        UpdatePassInfo result;
        result.z_position = update_state.old_z_position;

        auto& collision = result.collision;
        collision.valid_position = old_position;

        const auto& collision_frame = rotate_subject ? update_state.new_frame : update_state.old_frame;

        utility::LinePlottingRange position_range(old_position, new_position);
        collision.time_point.step = 0;
        collision.time_point.total_steps = position_range.step_count();

        auto collides = [&](std::uint32_t z_level, Vector2i position)
        {
          if (test_scenery_collision(collision_frame, scenery.frame(z_level), position)) return true;

          for (const auto& object_state : object_states)
          {
            if (z_level != object_state.z_level) continue;

            if (test_collision(collision_frame, object_state.old_frame,
                               position, object_state.current_position))
              return true;
          }

          return false;
        };

        auto level_transition = [&](std::uint32_t z_level, std::uint32_t new_z_level,
                                    Vector2i position)
        {
          if (z_level < new_z_level)
          {
            for (; z_level != new_z_level; ++z_level)
            {
              if (collides(z_level + 1, position)) return z_level;
            }
          }

          if (z_level > new_z_level)
          {
            for (; z_level != new_z_level; --z_level)
            {
              if (collides(z_level - 1, position)) return z_level;
            }
          }

          return z_level;
        };

        // First, attempt to move towards the projected z position.
        auto old_z_level = static_cast<std::uint32_t>(update_state.old_z_position);
        auto new_z_level = static_cast<std::uint32_t>(update_state.new_z_position);

        auto z_level = level_transition(old_z_level, new_z_level, old_position);
        if (z_level != old_z_level)
        {
          result.z_position = static_cast<double>(z_level);
        }

        auto scenery_frame = scenery.frame(z_level);

        for (auto position : position_range)
        {
          if (position.x < 0 || position.x >= world_size.x ||
              position.y < 0 || position.y >= world_size.y) continue;          

          if (auto collision_point = test_scenery_collision(collision_frame, scenery_frame, position))
          {
            collision.subject_state = &update_state;
            collision.collided = true;
            collision.stuck = old_position == position;
            collision.rotate = !collision.stuck;
            collision.point = collision_point.point;
            collision.collision_position = position;
            return result;
          }

          for (const auto& object_state : object_states)
          {
            if (z_level != object_state.z_level) continue;

            if (auto collision_point = test_collision(collision_frame, object_state.old_frame,
                                                      position, object_state.current_position))
            {
              collision.collided = true;
              collision.stuck = position == old_position;
              collision.rotate = collision.stuck && rotate_subject;

              collision.collision_position = position;

              collision.point = collision_point.point;
              collision.subject_state = &update_state;
              collision.object_state = &object_state;
              return result;
            }
          }

          auto new_z_level = z_level;
          const auto& terrain = world_obj.terrain_at(new_position);

          // See if we have a sub-terrain.
          const auto sub_terrain = terrain_library.sub_terrain(terrain.id, z_level);

          if (sub_terrain != 0)
          {
            // And if we do, transition to the top of the sub-terrain range.
            auto level_range = terrain_library.sub_level_range(terrain.id, z_level);

            new_z_level = level_range.second;
          }

          if (new_z_level != z_level)
          {
            // If the z level changed, attempt a z level transition and perform the
            // necessary collision tests.
            z_level = level_transition(z_level, new_z_level, position);
            result.z_position = static_cast<double>(z_level);
            scenery_frame = scenery.frame(z_level);
          }

          collision.valid_position = position;
          ++collision.time_point.step;
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
      CollisionAvoidance find_collision_avoidance(const World& world_obj,
                                                  const EntityUpdateState& update_state,
                                                  const resources::CollisionMask& scenery,
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
          auto offset = vector2_cast<std::int32_t>(attempt.offset);
          auto update_pass = entity_update_pass(world_obj, update_state,
                                                scenery, object_states,
                                                offset, rotate_subject);

          attempt.collision = update_pass.collision;
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
      CollisionAvoidance find_collision_avoidance(const World& world_obj,
                                                  const EntityUpdateState& update_state,
                                                  const resources::CollisionMask& scenery,
                                                  const ObjectStateRange& object_states,
                                                  const CollisionInfo& default_collision)
      {
        auto avoidance = find_collision_avoidance(world_obj, update_state, scenery,
                                                  object_states, true, default_collision);

        if (avoidance.collision && update_state.old_frame.row_begin(0) != update_state.new_frame.row_begin(0))
        {
          auto unrotated = find_collision_avoidance(world_obj, update_state, scenery,
                                                    object_states, false, default_collision);

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

    void World::update(std::uint32_t frame_duration, world::EventInterface& event_interface)
    {
      double fd = frame_duration * 0.001;

      auto max_corner = vector2_cast<std::int32_t>(world_size()) - make_vector2(1, 1);
      
      entity_update_state_.clear();
      for (auto* car : cars_)
      {
        auto old_position = car->position();
        auto old_rotation = car->rotation();

        auto z_level = car->z_level();

        auto terrain_func = [=](Vector2d position) -> decltype(auto)
        {
          auto x = utility::clamp(static_cast<std::int32_t>(position.x), 0, max_corner.x);
          auto y = utility::clamp(static_cast<std::int32_t>(position.y), 0, max_corner.y);

          return terrain_at(make_vector2(x, y), z_level);
        };

        car->update(terrain_func, fd);

        auto new_position = accomodate_position(car->position() + fd * car->velocity());
        auto new_rotation = radians(car->rotation().radians() + fd * car->rotating_speed());
        auto new_z_position = accomodate_z_position(car->z_position() + fd * car->z_speed());

        entity_update_state_.push_back({
          car, old_rotation, new_rotation, old_position, new_position,
          car->z_position(), new_z_position, car->z_level(),
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
          return intersects(entry.bounding_box, bounding_box);
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

        auto pass_info = detail::entity_update_pass(*this, update_state, collision_mask_,
                                                    object_range, {}, true);

        auto& collision = pass_info.collision;
        collision.collided = false;
        if (collision)
        {
          if (previously_collided_[entity_id])
          {
            auto avoidance = detail::find_collision_avoidance(*this, update_state,
                                                              collision_mask_, object_range, collision);
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
            auto object_id = collision.object_state->entity->entity_id();
            previously_collided_[object_id] = true;
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

        entity->set_z_position(pass_info.z_position);
        update_state.current_position = vector2_cast<std::int32_t>(entity->position());
        update_state.z_level = entity->z_level();

        {
          const auto& terrain = terrain_at(update_state.current_position);
          auto terrain_level = [&]() -> std::uint32_t
          {
            const auto& terrain_library = track_.terrain_library();
            for (auto z_level = update_state.z_level; z_level != 0; --z_level)
            {
              if (auto sub_terrain = terrain_library.sub_terrain(terrain.id, z_level))
              {
                return terrain_library.sub_level_range(terrain.id, z_level).second;
              }
            }  

            return 0;
          }();          

          // If we are not directly on top of a terrain, we need to set the flying state.
          double hover_distance = pass_info.z_position - static_cast<double>(terrain_level);
          entity->set_hover_distance(hover_distance);

          if (std::abs(hover_distance) < 0.0001) entity->set_z_speed(0.0);
        }

        previously_collided_[entity_id] = collision.collided;
        bounding_box_cache_.resize(bb_cache_size);

        auto cp_hit_callback = [&](const ControlPoint& point, double time_point)
        {
          auto frame_offset = static_cast<std::uint32_t>(frame_duration * time_point);

          event_interface.on_control_point_hit(entity, point, frame_offset);
        };

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

    double World::accomodate_z_position(double z_position) const
    {
      if (z_position < 0.0) z_position = 0.0;

      auto max = static_cast<double>(track_.height_level_count() - 1);
      if (z_position > max) z_position = max;

      return z_position;
    }

    const resources::Track& World::track() const noexcept
    {
      return track_;
    }

    const resources::TerrainDefinition& World::terrain_at(Vector2i position) const
    {
      auto terrain_id = pattern_(position.x, position.y);
      const auto& terrain_library = track().terrain_library();
      return terrain_library.terrain(terrain_id);
    }

    const resources::TerrainDefinition& World::terrain_at(Vector2<double> position) const
    {
      return terrain_at(vector2_cast<std::int32_t>(position));
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