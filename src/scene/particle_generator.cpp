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
#include "utility/transform.hpp"

#include <algorithm>

namespace ts
{
  namespace scene
  {
    ParticleGenerator::ParticleGenerator(const world::World* world_ptr, const ParticleSettings& settings)
      : world_(world_ptr),
      settings_(settings)
    {
      auto num_levels = world_ptr->track().height_level_count();
      particle_info_.resize(settings.max_particles * num_levels);
      level_info_.resize(num_levels);

      std::size_t index = 0;
      for (auto& level_info : level_info_)
      {
        level_info.index = index;
        level_info.base_index = index;
        index += settings.max_particles;
      }
    }

    std::size_t ParticleGenerator::level_count() const
    {
      return level_info_.size();
    }

    std::size_t ParticleGenerator::particle_count(std::size_t level) const
    {
      return level_info_[level].count;
    }

    const ParticleGenerator::ParticleInfo* ParticleGenerator::particle_info(std::size_t level) const
    {
      return particle_info_.data() + level_info_[level].index;
    }

    std::size_t ParticleGenerator::max_particles_per_level() const
    {
      return settings_.max_particles;
    }

    void ParticleGenerator::add_particle(LevelInfo& level_info, const ParticleInfo& particle_info)
    {
      if (level_info.index + level_info.count + 1 >= max_particles_per_level())
      {        
        std::copy_n(particle_info_.begin() + level_info.index, level_info.count,
                    particle_info_.begin() + level_info.base_index);

        level_info.index = level_info.base_index;
      }

      particle_info_[level_info.index + level_info.count] = particle_info;
      ++level_info.count;
    }

    void ParticleGenerator::update(std::uint32_t frame_duration)
    {
      const auto fd = frame_duration * 0.001;
      tick_counter_ += frame_duration;

      for (auto& level_info : level_info_)
      {
        auto level_start = particle_info_.begin() + level_info.index;
        auto level_end = level_start + level_info.count;

        // Remove the "expired" particles 
        auto it = std::find_if(level_start, level_end,
                               [ticks = tick_counter_](const ParticleInfo& particle_info)
        {
          return particle_info.end_ticks >= ticks;
        });

        // That is, we just shift the pointer up a bit.
        auto removal_count = static_cast<std::size_t>(std::distance(level_start, it));
        level_info.index += removal_count;
        level_info.count -= removal_count;
      }

      std::uniform_real_distribution<double> chance_dist(0.0, settings_.max_effect_speed);
      std::uniform_real_distribution<double> position_dist(-settings_.position_variance, settings_.position_variance);
      std::uniform_real_distribution<double> color_dist(-settings_.color_variance * 0.5, 
                                                        settings_.color_variance * 0.5);
      std::uniform_real_distribution<double> size_dist(settings_.min_size, settings_.max_size);

      const auto max_particles = max_particles_per_level();
      using utility::random_number;

      auto generate_position = [=](auto car_position)
      {
        auto offset = make_vector2(random_number(position_dist), random_number(position_dist)) * 0.5;
        return vector2_cast<float>(car_position + offset);        
      };

      auto generate_radius = [=]()
      {
        return static_cast<float>(random_number(size_dist) * 0.5);
      };

      auto generate_color = [=](auto base_color)
      {
        auto color = base_color;
        auto color_variance = std::max(std::min(random_number(color_dist), 1.0), -1.0);

        color.r += static_cast<int>(color.r * color_variance);
        color.g += static_cast<int>(color.g * color_variance);
        color.b += static_cast<int>(color.b * color_variance);
        color.a = 255;
        return color;
      };

      // Loop through all cars
      for (const auto& car : world_->cars())
      {
        auto level = car.z_level();
        auto& level_info = level_info_[level];

        auto traction = 1.0f;

        const auto& terrain = world_->terrain_at(car.position(), level);
        if (terrain.roughness >= 0.001 || (terrain.skid_mark && traction < 0.8))
        {
          auto rotation = static_cast<float>(car.rotation().radians());
          auto sin = std::sin(rotation), cos = std::cos(rotation);

          auto speed = std::min(magnitude(car.velocity()), settings_.max_effect_speed);
          /*
          for (auto tyre_position : car.tyre_positions())
          {
            auto transformed_tyre_pos = transform_point(vector2_cast<float>(tyre_position), sin, cos);

            // If we roll a number below the "chance factor", add a new particle.
            // The probability of success is positively affected by car speed, frame duration and terrain roughness.

            double roll = terrain.roughness >= 0.001 ?
              random_number(chance_dist) / (speed * fd * terrain.roughness) :
              random_number(chance_dist) * car.traction() / (speed * fd);

            Colorb base_color = { 150, 150, 150, 100 }; // Smoke color
            if (terrain.roughness >= 0.001)
            {
              base_color = terrain.color;
              base_color.r = (base_color.r * 225) / 256;
              base_color.g = (base_color.g * 225) / 256;
              base_color.b = (base_color.b * 225) / 256;
              base_color.a = 255;
            }

            for (; roll < settings_.chance_factor && level_info.count < max_particles; roll += settings_.chance_factor)
            {
              ParticleInfo info;
              info.position = generate_position(car.position()) + transformed_tyre_pos;
              info.radius = generate_radius();
              info.color = generate_color(base_color);
              info.end_ticks = tick_counter_ + settings_.display_time;


              add_particle(level_info, info);
            }
          }
          */
        }
      }
    }
  }
}