/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "entity.hpp"

#include "resources/collision_shape.hpp"

#include "utility/math_utilities.hpp"

#include <chipmunk/chipmunk.h>

namespace ts
{
  namespace world
  {

#define BODY_PTR static_cast<cpBody*>(physics_body_.get())

    void PhysicsBodyDeleter::operator()(void* body) const
    {
      cpBodyFree(static_cast<cpBody*>(body));
    }

    cpShape* create_shape(cpBody* body, const resources::collision_shapes::Circle& circle)
    {
      return cpCircleShapeNew(body, circle.radius, { circle.center.x, circle.center.y });      
    }

    cpShape* create_shape(cpBody* body, const resources::collision_shapes::Polygon& poly)                      
    {
      boost::container::small_vector<cpVect, 16> vertices;
      for (auto& p : poly.points)
      {
        vertices.push_back({ p.position.x, p.position.y });
      }     

      return cpPolyShapeNew(body, vertices.size(), vertices.data(), cpTransformIdentity, 0.0);
    }

    auto create_physics_body(const resources::CollisionShape& shape, double mass, double moment)
    {
      mass = std::max(mass, 1.0);
      moment = std::max(moment, 50.0);

      auto body = cpBodyNew(mass, mass * moment);
      std::unique_ptr<void, PhysicsBodyDeleter> owned_body(body);

      if (!shape.sub_shapes.empty())
      {
        double moment = 0.0;
        for (auto& sub_shape : shape.sub_shapes)
        {
          auto shape = boost::apply_visitor([&](auto& s)
          {
            return create_shape(body, s);
          }, sub_shape.data);

          cpShapeSetElasticity(shape, sub_shape.bounciness);
        }
      }
      
      return owned_body;
    }

    Entity::Entity(EntityId entity_id, EntityType entity_type, 
                   const resources::CollisionShape& collision_shape, double mass, double moment)
      : entity_id_(entity_id),
        entity_type_(entity_type),
        physics_body_(create_physics_body(collision_shape, mass, moment))
    {      
    }

    Vector2d Entity::position() const
    {
      auto pos = cpBodyGetPosition(BODY_PTR);
      return{ pos.x, pos.y };
    }

    Vector2d Entity::velocity() const
    {
      auto vel = cpBodyGetVelocity(BODY_PTR);
      return{ vel.x, vel.y };
    }

    Rotation<double> Entity::rotation() const
    {
      return radians(cpBodyGetAngle(BODY_PTR));     
    }

    double Entity::angular_velocity() const
    {
      return cpBodyGetAngularVelocity(BODY_PTR);
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

    void Entity::set_position(Vector2d position)
    {
      cpBodySetPosition(BODY_PTR, { position.x, position.y });
    }

    void Entity::set_velocity(Vector2d velocity)
    {
      cpBodySetVelocity(BODY_PTR, { velocity.x, velocity.y });
    }

    void Entity::set_rotation(Rotation<double> r)
    {
      cpBodySetAngle(BODY_PTR, r.radians());
    }

    void Entity::set_angular_velocity(double angular_velocity)
    {
      cpBodySetAngularVelocity(BODY_PTR, angular_velocity);
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

    double Entity::mass() const
    {
      return cpBodyGetMass(BODY_PTR);
    }

    Vector2d Entity::center_of_mass() const
    {
      auto cog = cpBodyGetCenterOfGravity(BODY_PTR);
      return{ cog.x, cog.y };
    }

    void Entity::set_mass(double mass)
    {
      cpBodySetMass(BODY_PTR, mass);
    }

    void Entity::update_z_speed(double frame_duration)
    {
      if (is_flying())
      {
        z_speed_ -= 1.5 * frame_duration;
      }      
    }

    void Entity::apply_force(Vector2d force, Vector2d point)
    {
      cpBodyApplyForceAtLocalPoint(BODY_PTR, { force.x, force.y }, { point.x, point.y });
    }
  }
}