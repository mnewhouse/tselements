/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "scene_loader.hpp"
#include "scene.hpp"
#include "scene_components.hpp"
#include "render_scene.hpp"

#include "track_scene_generator.hpp"
#include "dynamic_scene_generator.hpp"

#include "stage/stage.hpp"

#include "game/loading_thread.hpp"

namespace ts
{
  namespace scene
  {
    static auto create_particle_generator(const stage::Stage& stage)
    {
      return ParticleGenerator(&stage.world(), ParticleSettings());
    }

    static auto make_sound_effect_controller()
    {
      return SoundEffectController(12);
    }

    static auto make_car_sound_controller(const stage::Stage& stage)
    {
      const auto& stage_desc = stage.stage_description();
      CarSoundController car_sound_controller;

      for (const auto& model : stage_desc.car_models)
      {
        // Register all the models
        car_sound_controller.register_car_model(model);
      }

      for (const auto& instance : stage_desc.car_instances)
      {
        // And all the instances.
        if (auto car = stage.world().find_car(instance.instance_id))
        {
          car_sound_controller.register_car(instance.model_id, car);
        }        
      }

      return car_sound_controller;
    }

    SceneLoader::SceneLoader(game::LoadingThread* loading_thread)
      : loading_thread_(loading_thread)
    {}

    void SceneLoader::async_load_scene(const stage::Stage* stage_ptr)
    {
      auto loading_func = [this, stage_ptr]()
      {
        return this->load(stage_ptr);
      };      

      set_loading(true);
      scene_future_ = loading_thread_->async_task(loading_func);
    }
  
    Scene SceneLoader::load(const stage::Stage* stage_ptr)
    {
      return Scene(load_scene_components(stage_ptr), RenderScene(generate_track_scene(stage_ptr->track())));
    }

    bool SceneLoader::is_ready() const
    {
      return scene_future_ && 
        scene_future_->wait_for(std::chrono::nanoseconds(0)) == std::future_status::ready;
    }

    Scene SceneLoader::get_result()
    {
      set_loading(false);
      return scene_future_->get();
    }

    SceneComponents load_scene_components(const stage::Stage* stage_ptr)
    {
      return SceneComponents
      {
        stage_ptr,
        generate_dynamic_scene(*stage_ptr),
        create_particle_generator(*stage_ptr),
        make_car_sound_controller(*stage_ptr),
        make_sound_effect_controller()
      };
    }
  }
}