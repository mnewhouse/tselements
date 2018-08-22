/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
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

      std::uint32_t index = 0;
      for (auto& level_info : level_info_)
      {
        level_info.index = 0;
        level_info.count = 0;
        level_info.base_index = index;
        index += settings.max_particles;
      }
    }

    std::uint32_t ParticleGenerator::level_count() const
    {
      return level_info_.size();
    }

    std::uint32_t ParticleGenerator::particle_count(std::uint32_t level) const
    {
      return level_info_[level].count;
    }

    std::uint32_t ParticleGenerator::particle_start_index(std::uint32_t level) const
    {
      return level_info_[level].index;
    }

    const ParticleGenerator::ParticleInfo* ParticleGenerator::particle_info(std::uint32_t level) const
    {
      return particle_info_.data() + level_info_[level].base_index;
    }

    std::uint32_t ParticleGenerator::max_particles_per_level() const
    {
      return settings_.max_particles;
    }

    void ParticleGenerator::add_particle(LevelInfo& level_info, const ParticleInfo& particle_info)
    {
      auto idx = level_info.index + level_info.count;
      if (idx >= max_particles_per_level())
      {
        idx -= max_particles_per_level();
      }

      particle_info_[idx] = particle_info;
      ++level_info.count;
    }

    void ParticleGenerator::update(std::uint32_t frame_duration)
    {
      const auto fd = frame_duration * 0.001;
      tick_counter_ += frame_duration;      

      for (auto& level_info : level_info_)
      {
        auto level_start = particle_info_.begin() + level_info.base_index;

        // Remove the "expired" particles   
        auto& idx = level_info.index;
        auto& count = level_info.count;
        while (count != 0 && tick_counter_ >= level_start[idx].end_ticks)
        {
          ++idx;
          --count;

          if (idx == max_particles_per_level()) idx = 0;          
        }       
      }

      std::uniform_real_distribution<double> chance_dist(0.0, 10.0);
      std::uniform_real_distribution<double> position_dist(-settings_.position_variance, settings_.position_variance);
      std::uniform_real_distribution<double> color_dist(-settings_.color_variance * 0.5, 
                                                        settings_.color_variance * 0.5);
      std::uniform_real_distribution<double> size_dist(settings_.min_size, settings_.max_size);
      std::uniform_real_distribution<double> smoke_size_dist(settings_.min_smoke_size, settings_.max_smoke_size);

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

      auto generate_smoke_radius = [=]()
      {
        return static_cast<float>(random_number(smoke_size_dist) * 0.5);
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
        const auto& handling_state = car.handling_state();

        for (const auto& ws : handling_state.wheel_states)
        {          
          if (ws.terrain_roughness < 0.001 && ws.slide_ratio < 0.1) continue;

          bool smoke = ws.terrain_roughness < 0.001;
          auto speed = std::min(settings_.max_effect_speed, ws.speed);

          // If we roll a number below the "chance factor", add a new particle.
          // The probability of success is positively affected by car speed, frame duration and terrain roughness.

          

          auto chance_factor = !smoke ?
            settings_.chance_factor * speed * fd * std::min(ws.terrain_roughness, 1.0) :
            settings_.smoke_chance_factor * speed * fd * ws.slide_ratio;

          double roll = random_number(chance_dist);

          Colorb base_color = { 150, 150, 150, 100 }; // Smoke color
          if (!smoke)
          {
            base_color = ws.terrain_color;
            base_color.r = (base_color.r * 225) >> 8;
            base_color.g = (base_color.g * 225) >> 8;
            base_color.b = (base_color.b * 225) >> 8;
            base_color.a = 255;
          }

          for (; roll < chance_factor && level_info.count < max_particles; roll += chance_factor)
          {
            ParticleInfo info;
            info.position = generate_position(ws.pos);
            info.radius = smoke ? generate_smoke_radius() : generate_radius();
            info.color = generate_color(base_color);
            info.end_ticks = tick_counter_ + settings_.display_time;

            add_particle(level_info, info);
          }
        }
      }
    }
  }
}