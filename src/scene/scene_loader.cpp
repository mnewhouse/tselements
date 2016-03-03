/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "scene_loader.hpp"
#include "scene.hpp"
#include "track_scene_generator.hpp"
#include "dynamic_scene_generator.hpp"
#include "particle_generator.hpp"

#include "stage/stage.hpp"

#include "game/loading_thread.hpp"

namespace ts
{
  namespace scene
  {
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
      Scene scene_obj;
      scene_obj.stage_ptr = stage_ptr;
      scene_obj.track_scene = generate_track_scene(stage_ptr->track());
      scene_obj.dynamic_scene = generate_dynamic_scene(*stage_ptr);

      scene_obj.particle_generator = std::make_unique<ParticleGenerator>(&stage_ptr->world(), ParticleSettings());
      scene_obj.scene_renderer = SceneRenderer(scene_obj.track_scene.get(), scene_obj.dynamic_scene.get(),
                                               scene_obj.particle_generator.get());

      const auto& stage_desc = stage_ptr->stage_description();
      for (const auto& model : stage_desc.car_models)
      {
        scene_obj.car_sound_controller.register_car_model(model);
      }

      for (const auto& instance : stage_desc.car_instances)
      {
        auto car = stage_ptr->world().find_car(instance.instance_id);
        scene_obj.car_sound_controller.register_car(instance.model_id, car);
      }
      
      return scene_obj;
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
  }
}