/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

namespace ts
{
  namespace components
  {
    // class template GenericState
    // Provides a generic interface for game states. The event and context 
    // types as seen below need to be specified in the traits type.
    template <typename TraitsType>
    class GenericState
    {
    public:
      using game_context = typename TraitsType::game_context;
      using event_type = typename TraitsType::event_type;
      using update_context = typename TraitsType::update_context;
      using render_context = typename TraitsType::render_context;

      GenericState(game_context context)
        : game_context_(std::move(context))
      {
      }

      virtual ~GenericState() = default;

      virtual void process_event(const event_type& event) {}
      virtual void update(const update_context& context) {}
      virtual void render(const render_context& context) const {}

      virtual void on_activate() {}
      virtual void on_deactivate() {}

      const game_context& context() const { return game_context_; }

    private:
      game_context game_context_;
    };

    template <typename StateType>
    void activate(GenericState<StateType>& state)
    {
      state.on_activate();
    }

    template <typename StateType>
    void deactivate(GenericState<StateType>& state)
    {
      state.on_deactivate();
    }
  }
}
