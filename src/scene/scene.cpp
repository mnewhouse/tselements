/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "scene.hpp"
#include "scene_components.hpp"
#include "render_scene.hpp"
#include "viewport_arrangement.hpp"

#include "world/world_messages.hpp"

namespace ts
{
  namespace scene
  {
    struct Scene::Impl
      : SceneComponents
    {
      Impl(SceneComponents components)
        : SceneComponents(std::move(components))
      {
      }
    };

    

    Scene::Scene(SceneComponents components)
      : impl_(std::make_unique<Impl>(std::move(components)))
    {

    }

    Scene::Scene() = default;
    Scene::~Scene() = default;

    Scene::Scene(Scene&&) = default;
    Scene& Scene::operator=(Scene&&) = default;

    const stage::Stage& Scene::stage() const
    {
      return *impl_->stage_ptr_;
    }

    void Scene::render(const ViewportArrangement& viewport_arrangement, Vector2i screen_size,
                       double frame_progress) const
    {
      for (std::size_t viewport_id = 0; viewport_id != viewport_arrangement.viewport_count(); ++viewport_id)
      {
        const auto& viewport = viewport_arrangement.viewport(viewport_id);
        impl_->render_scene_.render(viewport, screen_size, frame_progress);
      }
    }

    void Scene::update_stored_state()
    {
      impl_->dynamic_scene_.update_entity_positions();
    }

    void Scene::update(std::uint32_t frame_duration)
    {
      impl_->render_scene_.update_entities(impl_->dynamic_scene_, frame_duration);

      impl_->car_sound_controller_.update(frame_duration);
      impl_->particle_generator_.update(frame_duration);
    }

    void Scene::handle_collision(const world::messages::SceneryCollision& collision)
    {
      //impl_->sound_effect_controller_.play_collision_sound(*collision.entity, collision.collision);
    }

    void Scene::handle_collision(const world::messages::EntityCollision& collision)
    {
      //impl_->sound_effect_controller_.play_collision_sound(*collision.subject, *collision.object,
      //                                                    collision.collision);
    }

    SceneComponents Scene::release()
    {
      return std::move(*impl_);
    }
  }
}