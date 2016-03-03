/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef GENERIC_STATE_HPP_6678
#define GENERIC_STATE_HPP_6678

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

      virtual void process_event(const event_type& event) {};
      virtual void update(const update_context& context) {};
      virtual void render(const render_context& context) const {};

      const game_context& context() const { return game_context_; }

    private:
      game_context game_context_;
    };
  }
}

#endif