/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "entity.hpp"

#include "utility/math_utilities.hpp"

namespace ts
{
  namespace world
  {
    Entity::Entity(EntityId entity_id, EntityType entity_type, std::shared_ptr<resources::CollisionMask> collision_mask)
      : entity_id_(entity_id),
        entity_type_(entity_type),
        collision_mask_(std::move(collision_mask))
    {
    }

    Vector2<double> Entity::position() const
    {
      return raw_state_.position / 32768.0;
    }

    Vector2<double> Entity::velocity() const
    {
      auto result = make_vector2((raw_state_.velocity.x & ~(1U << 31)) / 32768.0,
                                 (raw_state_.velocity.y & ~(1U << 31)) / 32768.0);

      if (raw_state_.velocity.x & (1U << 31)) result.x = -result.x;
      if (raw_state_.velocity.y & (1U << 31)) result.y = -result.y;

      return result;
    }

    Rotation<double> Entity::rotation() const
    {
      auto rad = (raw_state_.rotation & ~(1U << 23)) / 8388607.0 * Rotation<double>::pi;
      if (raw_state_.rotation & (1U << 23)) rad = -rad;
      
      return radians(rad);
    }

    double Entity::rotating_speed() const
    {
      auto result = (raw_state_.rotating_speed & ~(1U << 23)) / 32768.0;
      if (raw_state_.rotating_speed & (1U << 23)) result = -result;

      return result;
    }

    double Entity::z_speed() const
    {
      auto result = (raw_state_.z_speed & ~(1U << 15)) / 256.0;
      if (raw_state_.z_speed & (1U << 15)) result = -result;

      return result;
    }

    double Entity::z_position() const
    {
      return raw_state_.z_position / 1024.0;
    }

    bool Entity::is_flying() const
    {
      return std::abs(hover_distance_) > 0.0001;
    }

    void Entity::set_hover_distance(double hover_distance)
    {
      hover_distance_ = hover_distance;
    }

    double Entity::hover_distance() const
    {
      return hover_distance_;
    }

    std::uint32_t Entity::z_level() const
    {
      return static_cast<std::uint32_t>(z_position_);
    }

    void Entity::set_position(Vector2<double> position)
    {
      position.x = utility::clamp(position.x, 0.0, 32767.0);
      position.y = utility::clamp(position.y, 0.0, 32767.0);

      raw_state_.position = vector2_cast<std::uint32_t>(position * 32768.0);
    }

    void Entity::set_velocity(Vector2<double> velocity)
    {
      velocity.x = utility::clamp(velocity.x, -32767.0, 32767.0);
      velocity.y = utility::clamp(velocity.y, -32767.0, 32767.0);

      raw_state_.velocity.x = static_cast<std::uint32_t>(std::abs(velocity.x) * 32768.0);
      raw_state_.velocity.y = static_cast<std::uint32_t>(std::abs(velocity.y) * 32768.0);

      if (velocity.x < 0.0) raw_state_.velocity.x |= (1U << 31);
      if (velocity.y < 0.0) raw_state_.velocity.y |= (1U << 31);
    }

    void Entity::set_rotation(Rotation<double> rotation)
    {
      rotation.normalize();

      raw_state_.rotation = static_cast<std::uint32_t>(std::abs(rotation.radians() / Rotation<double>::pi) * 8388607.0);

      if (rotation.radians() < 0) raw_state_.rotation |= (1U << 23);
    }

    void Entity::set_rotating_speed(double rotating_speed)
    {
      rotating_speed = utility::clamp(rotating_speed, -255.0, 255.0);
      
      raw_state_.rotating_speed = static_cast<std::uint32_t>(std::abs(rotating_speed) * 32768.0);

      if (rotating_speed < 0.0)
      {
        raw_state_.rotating_speed |= (1U << 23);
      }
    }

    void Entity::set_z_speed(double z_speed)
    {
      z_speed = utility::clamp(z_speed, -255.0, 255.0);

      raw_state_.z_speed = static_cast<std::uint16_t>(std::abs(z_speed * 256.0));

      if (z_speed < 0.0)
      {
        raw_state_.z_speed |= (1U < 15);
      }
    }

    void Entity::set_z_position(double z)
    {
      z = utility::clamp(z, 0.0, 63.0);

      raw_state_.z_position = static_cast<std::uint16_t>(z * 1024.0);
    }

    EntityType Entity::type() const
    {
      return entity_type_;
    }

    EntityId Entity::entity_id() const
    {
      return entity_id_;
    }

    const resources::CollisionMask* Entity::collision_mask() const
    {
      return collision_mask_.get();
    }

    double Entity::bounciness() const
    {
      return bounciness_;
    }

    void Entity::set_bounciness(double bounciness)
    {
      bounciness_ = bounciness;
    }

    double Entity::mass() const
    {
      return mass_;
    }

    void Entity::set_mass(double mass)
    {
      mass_ = mass;
    }

    void Entity::update_z_speed(double frame_duration)
    {
      if (is_flying())
      {
        z_speed_ -= 1.5 * frame_duration;
      }      
    }
  }
}