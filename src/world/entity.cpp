/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "entity.hpp"

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
      return position_;
    }

    Vector2<double> Entity::velocity() const
    {
      return velocity_;
    }

    Rotation<double> Entity::rotation() const
    {
      return rotation_;
    }

    double Entity::rotating_speed() const
    {
      return rotating_speed_;
    }

    double Entity::z_position() const
    {
      return z_position_;
    }

    std::uint32_t Entity::z_level() const
    {
      return static_cast<std::uint32_t>(z_position_);
    }

    void Entity::set_position(Vector2<double> position)
    {
      position_ = position;
    }

    void Entity::set_velocity(Vector2<double> velocity)
    {
      velocity_ = velocity;
    }

    void Entity::set_rotation(Rotation<double> rotation)
    {
      rotation_ = rotation;
    }

    void Entity::set_rotating_speed(double rotating_speed)
    {
      rotating_speed_ = rotating_speed;
    }

    void Entity::set_z_position(double z)
    {
      z_position_ = z;
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
  }
}