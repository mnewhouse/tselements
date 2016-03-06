/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "particle_generator.hpp"

#include "world/world.hpp"
#include "world/car.hpp"

#include "resources/terrain_definition.hpp"

#include "utility/random.hpp"

#include <algorithm>

namespace ts
{
  namespace scene
  {
    namespace detail
    {
      template <typename OutIt>
      void generate_vertices(Vector2<float> position, float size, Colorb color, OutIt out)
      {
        ParticleVertex vertex;
        vertex.color = color;

        auto half_size = size * 0.5f;

        // Let's generate six vertices which together make up a perfect hexagon.

        auto angled_offset = make_vector2(half_size * 0.86602540378f, -half_size * 0.5f); // transformed by 30 degrees
        vertex.position.x = position.x;
        vertex.position.y = position.y - half_size;
        *out++ = vertex;

        vertex.position = position + angled_offset;
        *out++ = vertex;

        vertex.position.y = position.y - angled_offset.y;
        *out++ = vertex;

        vertex.position.y = position.y + half_size;
        vertex.position.x = position.x;
        *out++ = vertex;

        vertex.position = position - angled_offset;
        *out++ = vertex;

        vertex.position.y = position.y + angled_offset.y;
        *out++ = vertex;
      }

      static constexpr std::uint32_t indices[] =
      {
        0, 1, 2,
        0, 2, 3,
        0, 3, 4,
        0, 4, 5
      };

      template <typename OutIt>
      void generate_indices(OutIt out)
      {
        std::copy(std::begin(indices), std::end(indices), out);
      }
    }

    ParticleGenerator::ParticleGenerator(const world::World* world_ptr, const ParticleSettings& settings)
      : world_(world_ptr),
        settings_(settings)    
    {
      vertices_.reserve(settings.max_particles * vertices_per_particle());
      particle_info_.reserve(settings.max_particles);
    }

    const ParticleVertex* ParticleGenerator::vertices() const
    {
      return vertices_.data();
    }

    std::size_t ParticleGenerator::vertex_count() const
    {
      return vertices_.size();
    }

    const std::uint32_t* ParticleGenerator::indices() const
    {
      return detail::indices;
    }

    std::size_t ParticleGenerator::particle_count() const
    {
      return particle_info_.size();
    }

    std::size_t ParticleGenerator::max_particles() const
    {
      return settings_.max_particles;
    }

    std::size_t ParticleGenerator::vertices_per_particle()
    {
      return 6;
    }

    std::size_t ParticleGenerator::indices_per_particle()
    {
      return 12;
    }      

    void ParticleGenerator::update(std::uint32_t frame_duration)
    {
      const auto fd = frame_duration * 0.001;
      tick_counter_ += frame_duration;

      // Remove the "expired" particles 
      auto it = std::find_if(particle_info_.begin(), particle_info_.end(),
                             [ticks = tick_counter_](const ParticleInfo& particle_info)
      {
        return particle_info.end_ticks >= ticks;
      });

      auto removal_count = std::distance(particle_info_.begin(), it);
      particle_info_.erase(particle_info_.begin(), it);
      vertices_.erase(vertices_.begin(), vertices_.begin() + removal_count * vertices_per_particle());

      std::uniform_real_distribution<double> chance_dist(0.0, settings_.max_effect_speed);
      std::uniform_real_distribution<double> position_dist(-settings_.position_variance, settings_.position_variance);
      std::uniform_real_distribution<double> color_dist(-settings_.color_variance * 0.5, 
                                                        settings_.color_variance * 0.5);
      std::uniform_real_distribution<double> size_dist(settings_.min_size, settings_.max_size);

      // Loop through all cars
      for (const auto& car : world_->cars())
      {
        const auto& terrain = world_->terrain_at(car.position(), car.z_level());
        if (terrain.roughness >= 0.001)
        {
          auto speed = std::min(magnitude(car.velocity()), settings_.max_effect_speed);

          // If we roll a number below the "chance factor", add a new particle.
          // The probability of success is positively affected by car speed, frame duration and terrain roughness.

          using utility::random_number;
          auto roll = random_number(chance_dist) / (speed * fd * terrain.roughness);
          for (; roll < settings_.chance_factor && particle_count() < max_particles(); roll += settings_.chance_factor)
          {
            auto offset = make_vector2(random_number(position_dist), random_number(position_dist)) * 0.5;
            auto position = vector2_cast<float>(car.position() + offset);
            auto size = static_cast<float>(random_number(size_dist));
            
            auto color = terrain.color;
            auto color_variance = std::max(std::min(random_number(color_dist), 1.0), -1.0);

            color.r += static_cast<int>(color.r * color_variance);
            color.g += static_cast<int>(color.g * color_variance);
            color.b += static_cast<int>(color.b * color_variance);
            color.a = 255;

            // Create a new particle
            ParticleInfo info;
            info.end_ticks = tick_counter_ + settings_.display_time;
            particle_info_.push_back(info);

            // Generate vertices
            auto vertex_index = vertices_.size();
            vertices_.resize(vertex_index + vertices_per_particle());
            detail::generate_vertices(position, size, color, vertices_.begin() + vertex_index);
          }
        }
      }
    }
  }
}