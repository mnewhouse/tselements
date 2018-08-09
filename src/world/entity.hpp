/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "entity_type.hpp"

#include "resources/collision_mask.hpp"

#include "utility/vector2.hpp"
#include "utility/rotation.hpp"

#include <boost/container/small_vector.hpp>

#include <memory>

namespace ts
{
  namespace resources
  {
    struct CollisionShape;
  }

  namespace world
  {
    using EntityId = std::uint16_t;
    class PhysicsSpace;

    struct PhysicsBodyDeleter
    {
      void operator()(void*) const;
    };

    // The Entity class is the base class of all dynamic world objects.
    // All of these entities share a few common properties, such as position, velocity and rotation,
    // and this class provides an interface to examine and modify said properties.
    class Entity
    {
    public:
      explicit Entity(EntityId entity_id, EntityType entity_type,
                      const resources::CollisionShape& collision_shape, double mass, double moment);

      virtual ~Entity() = default;

      void set_position(Vector2d position);
      Vector2d position() const;

      void set_velocity(Vector2d velocity);
      Vector2d velocity() const;

      double z_speed() const;
      void set_z_speed(double z_speed);

      void set_rotation(Rotation<double> rotation);
      Rotation<double> rotation() const;

      void set_angular_velocity(double rotation_speed);
      double angular_velocity() const;

      void set_z_position(double z_position);
      double z_position() const;
      std::uint32_t z_level() const;

      EntityType type() const;
      EntityId entity_id() const;

      double mass() const;
      void set_mass(double mass);

      Vector2d center_of_mass() const;
      void set_center_of_mass(Vector2d center);

      bool is_flying() const;
      void set_hover_distance(double hover_distance);
      double hover_distance() const;

      void update_z_speed(double frame_duration);

      struct RawState
      {
        Vector2<std::uint32_t> position = {};
        Vector2<std::uint32_t> velocity = {};

        std::uint32_t rotating_speed = 0;
        std::uint32_t rotation = 0;

        std::uint16_t z_speed = 0;
        std::uint16_t z_position = 0;
      };

      RawState raw_state() const;
      void load_raw_state(RawState state);

      void apply_force(Vector2d force, Vector2d point);

      Vector2d applied_force() const;
      double applied_torque() const;

    protected:
      void* physics_body();
      const void* physics_body() const;

    private:
      RawState raw_state_;

      Vector2<double> position_;
      Vector2<double> velocity_;
      Rotation<double> rotation_;
      double rotating_speed_ = 0.0;
      double z_speed_ = 0.0;
      double z_position_ = 0.0;
      double hover_distance_ = 0.0;
      
      EntityId entity_id_;
      EntityType entity_type_;

      double bounciness_ = 0.0;
      double mass_ = 100.0;

      friend PhysicsSpace;

      std::unique_ptr<void, PhysicsBodyDeleter> physics_body_;
    };
  }
}
