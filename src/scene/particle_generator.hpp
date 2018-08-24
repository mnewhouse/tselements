/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "utility/vertex.hpp"

#include <vector>

namespace ts
{
  namespace world
  {
    class World;
  }

  namespace scene
  {
    struct ParticleSettings
    {
      double min_size = 1.5;
      double max_size = 3.0;
      double min_smoke_size = 3.0;
      double max_smoke_size = 9.0;
      double chance_factor = 5.0;
      double smoke_chance_factor = 1.0;
      double max_effect_speed = 250.0;
      double position_variance = 3.0;
      double color_variance = 0.4;
      std::uint32_t display_time = 400;      
      std::uint32_t max_particles = 1024;
    };

    // The particle generator class generates particles for all cars in the game world,
    // if certain criteria are met. On rough terrains, particles will be shown, 
    // otherwise if the car is sliding, smoke will be shown. The particle's properties are
    // affected by the settings as they were passed to the constructor.
    class ParticleGenerator
    {
    public:
      explicit ParticleGenerator(const world::World* world, const ParticleSettings& particle_settings);

      void update(std::uint32_t frame_duration);

      std::uint32_t level_count() const;
      std::uint32_t max_particles_per_level() const;      

      struct ParticleInfo
      {
        Vector2f position;
        float radius;
        Colorb color;
        std::uint32_t end_ticks;
      };

      const ParticleInfo* particle_info(std::uint32_t level) const;
      std::uint32_t particle_count(std::uint32_t level) const;
      std::uint32_t particle_start_index(std::uint32_t level) const;

    private:
      struct LevelInfo
      {
        std::uint32_t base_index;
        std::uint32_t index;
        std::uint32_t count;
      };

      void add_particle(LevelInfo& level_info, const ParticleInfo& particle_info);

      const world::World* world_;
      ParticleSettings settings_;

      std::vector<ParticleInfo> particle_info_;
      std::vector<LevelInfo> level_info_;
      std::uint32_t tick_counter_ = 0;   
    };
  }
}
