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

    struct ParticleVertex
    {
      Vector2<float> position;
      Colorb color;
    };

    class ParticleGenerator
    {
    public:
      explicit ParticleGenerator(const world::World* world, const ParticleSettings& particle_settings);

      void update(std::size_t frame_duration);

      const ParticleVertex* vertices() const;
      std::size_t vertex_count() const;

      const std::uint32_t* indices() const;

      std::size_t particle_count() const;
      std::size_t max_particles() const;

      static std::size_t vertices_per_particle();
      static std::size_t indices_per_particle();

    private:
      const world::World* world_;
      ParticleSettings settings_;

      struct ParticleInfo
      {
        std::uint64_t end_ticks;
      };
      
      std::vector<ParticleVertex> vertices_;
      std::vector<ParticleInfo> particle_info_;
      std::uint64_t tick_counter_ = 0;   
    };
  }
}

#endif