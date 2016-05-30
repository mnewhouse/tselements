/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef PARTICLE_GENERATOR_HPP_5781923645197283
#define PARTICLE_GENERATOR_HPP_5781923645197283

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
      double min_size = 2.0;
      double max_size = 4.0;
      double chance_factor = 10.0;
      double max_effect_speed = 50.0;
      double position_variance = 2.0;
      double color_variance = 0.4;
      std::size_t display_time = 400;      
      std::size_t max_particles = 1024;
    };

    // The particle generator class generates particles for all cars in the game world,
    // if certain criteria are met. On rough terrains, particles will be shown, 
    // otherwise if the car is sliding, smoke will be shown. The particle's properties are
    // affected by the settings as it was passed to the constructor.
    class ParticleGenerator
    {
    public:
      explicit ParticleGenerator(const world::World* world, const ParticleSettings& particle_settings);

      void update(std::uint32_t frame_duration);

      std::size_t level_count() const;
      std::size_t max_particles_per_level() const;      

      struct ParticleInfo
      {
        Vector2f position;
        float radius;
        Colorb color;
        std::uint64_t end_ticks;
      };

      const ParticleInfo* particle_info(std::size_t level) const;
      std::size_t particle_count(std::size_t level) const;

    private:
      struct LevelInfo
      {
        std::size_t base_index;
        std::size_t index;
        std::size_t count;
      };

      void add_particle(LevelInfo& level_info, const ParticleInfo& particle_info);

      const world::World* world_;
      ParticleSettings settings_;


      
      std::vector<ParticleInfo> particle_info_;
      std::vector<LevelInfo> level_info_;
      std::uint64_t tick_counter_ = 0;   
    };
  }
}

#endif