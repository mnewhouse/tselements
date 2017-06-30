/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "entity_type.hpp"

#include "resources/collision_mask.hpp"

#include "utility/vector2.hpp"
#include "utility/rotation.hpp"

#include <memory>

namespace ts
{
  namespace world
  {
    using EntityId = std::uint16_t;

    // The Entity class is the base class of all dynamic world objects.
    // All of these entities share a few common properties, such as position, velocity and rotation,
    // and this class provides an interface to examine and modify said properties.
    class Entity
    {
    public:
      explicit Entity(EntityId entity_id, EntityType entity_type, 
                      std::shared_ptr<resources::CollisionMask> collision_mask);
      virtual ~Entity() = default;

      void set_position(Vector2<double> position);
      void set_position(double x, double y);
      Vector2<double> position() const;

      void set_velocity(Vector2<double> velocity);
      void set_velocity(double x, double y);
      Vector2<double> velocity() const;

      double z_speed() const;
      void set_z_speed(double z_speed);

      void set_rotation(Rotation<double> rotation);
      void set_rotation(double rotation, rotation_units::degrees_t);
      void set_rotation(double rotation, rotation_units::radians_t);
      Rotation<double> rotation() const;

      void set_rotating_speed(double rotation_speed);
      double rotating_speed() const;

      void set_z_position(double z_position);
      double z_position() const;
      std::uint32_t z_level() const;

      EntityType type() const;
      EntityId entity_id() const;

      const resources::CollisionMask* collision_mask() const;

      double bounciness() const;
      void set_bounciness(double bounciness);

      double mass() const;
      void set_mass(double mass);

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

      std::shared_ptr<resources::CollisionMask> collision_mask_;
    };
  }
}
