/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "action_state.hpp"

#include "client/key_settings.hpp"
#include "client/control_event_translator.hpp"
#include "client/client_viewport_arrangement.hpp"

#include "controls/control_center.hpp"

#include "resources/resource_store.hpp"
#include "resources/settings.hpp"

#include "scene/scene.hpp"
#include "scene/viewport.hpp"

namespace ts
{
  namespace client
  {
    template <typename MessageDispatcher>
    struct ActionState<MessageDispatcher>::Impl
    {
      explicit Impl(scene::Scene&& scene, 
                    controls::ControlCenter&& control_center,
                    const MessageDispatcher* message_dispatcher, UpdateInterface&& update_interface,
                    const KeySettings& key_settings);

      void render(const game::RenderContext& render_context);

      scene::Scene scene_;

      const MessageDispatcher* message_dispatcher_;
      controls::ControlCenter control_center_;
      ControlEventTranslator control_event_translator_;
      UpdateInterface update_interface_;
      ViewportArrangement viewport_arrangement_;
    };

    template <typename MessageDispatcher>
    ActionState<MessageDispatcher>::
      ActionState(const game_context& context, scene::Scene scene,
                  controls::ControlCenter control_center,
                  const MessageDispatcher* message_dispatcher, UpdateInterface update_interface)
      : GameState(context),
         impl_(std::make_unique<Impl>(std::move(scene),
                                      std::move(control_center),
                                      message_dispatcher, 
                                      std::move(update_interface),
                                      context.resource_store->settings().key_settings()))
    {
    }

    template <typename MessageDispatcher>
    void ActionState<MessageDispatcher>::render(const render_context& context) const
    {
      impl_->render(context);
    }

    template <typename MessageDispatcher>
    void ActionState<MessageDispatcher>::update(const update_context& context)
    {
      impl_->viewport_arrangement_.update_viewports();
      impl_->scene_.dynamic_scene->update_entity_positions();

      impl_->update_interface_.update(context);

      impl_->scene_.scene_renderer.update(context.frame_duration);
      impl_->scene_.car_sound_controller.update(context.frame_duration);
      impl_->scene_.particle_generator->update(context.frame_duration);
      
    }

    template <typename MessageDispatcher>
    void ActionState<MessageDispatcher>::process_event(const event_type& event)
    {
      impl_->control_event_translator_.translate_event(event, *impl_->message_dispatcher_);
    }

    template <typename MessageDispatcher>
    ActionState<MessageDispatcher>::
      Impl::Impl(scene::Scene&& scene,
                 controls::ControlCenter&& control_center,
                 const MessageDispatcher* message_dispatcher,
                 UpdateInterface&& update_interface,
                 const KeySettings& key_settings)
      : scene_(std::move(scene)),
        control_center_(std::move(control_center)),
        message_dispatcher_(message_dispatcher),
        update_interface_(std::move(update_interface)),
        control_event_translator_(scene_.stage_ptr, &control_center_, key_settings.key_mapping),
        viewport_arrangement_(16, { 0.0, 0.0, 1.0, 1.0 }, control_center_, *scene_.stage_ptr)
    {
    }

    template <typename MessageDispatcher>
    void ActionState<MessageDispatcher>::Impl::render(const game::RenderContext& render_context)
    {
      for (std::size_t index = 0, count = viewport_arrangement_.viewport_count(); index != count; ++index)
      {
        const auto& viewport = viewport_arrangement_.viewport(index);
        scene_.scene_renderer.render(viewport, render_context.screen_size, render_context.frame_progress);
      }
    }
  }
}