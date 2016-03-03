/*
 * The MIT License (MIT)
 *
 * TS Evolution
 * Copyright (c) 2015 Martin Newhouse
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef GENERIC_STATE_MACHINE_INL_178237
#define GENERIC_STATE_MACHINE_INL_178237

#include <algorithm>

namespace ts
{
  namespace components
  {
    template <typename StateType>
    StateMachine<StateType>::~StateMachine()
    {
      // First, destroy the active states in reverse order
      while (!state_stack_.empty())
      {
        state_map_.erase(state_stack_.back().index);

        state_stack_.pop_back();
      }

      // Then, just let it clean up after itself
    }

    template <typename StateType>
    template <typename ConcreteType, typename... Args>
    ConcreteType* StateMachine<StateType>::create_state(Args&&... args)
    {
      // Create the state
      ConcreteType* result = this->create_state<ConcreteType>(state_machine::deferred, std::forward<Args>(args)...);

      if (result)
      {
        // Activate it, if possible
        this->activate_state<ConcreteType>();
      }

      // And return the result.
      return result;
    }

    template <typename StateType>
    template <typename ConcreteType, typename... Args>
    ConcreteType* StateMachine<StateType>::create_state(state_machine::DeferredTag, Args&&... args)
    {
      std::type_index index = typeid(ConcreteType);

      // Only create it if there's no instance with the same type.
      auto instance_it = state_map_.find(index);
      if (instance_it == state_map_.end())
      {
        
        auto instance = std::make_unique<ConcreteType>(std::forward<Args>(args)...);
        auto pointer = instance.get();

        auto result = state_map_.insert(std::make_pair(index, std::move(instance)));
        if (result.second)
        {
          return pointer;
        }
      }

      return nullptr;
    }

    template <typename StateType>
    template <typename ConcreteType>
    void StateMachine<StateType>::destroy_state()
    {
      std::type_index index = typeid(ConcreteType);
      auto instance_it = state_map_.find(index);
      if (instance_it != state_map_.end())
      {
        auto guard = transition_guard();

        // Deactivate the state.
        Transition transition;
        transition.type = Destroy;
        transition.state_info.state = instance_it->second.get();;
        transition.state_info.index = index;

        state_transitions_.push_back(transition);
      }
    }

    template <typename StateType>
    template <typename ConcreteType>
    void StateMachine<StateType>::deactivate_state()
    {
      std::type_index index = typeid(ConcreteType);
      this->deactivate_state(index);
    }

    template <typename StateType>
    void StateMachine<StateType>::deactivate_state(std::type_index index)
    {
      auto instance_it = state_map_.find(index);
      if (instance_it != state_map_.end())
      {
        auto guard = transition_guard();

        // Deactivate the state.
        Transition transition;
        transition.type = Deactivate;
        transition.state_info.state = instance_it->second.get();;
        transition.state_info.index = index;
        state_transitions_.push_back(transition);
      }
    }

    template <typename StateType>
    template <typename ConcreteType>
    void StateMachine<StateType>::activate_state()
    {
      this->activate_state(typeid(ConcreteType));
    }

    template <typename StateType>
    void StateMachine<StateType>::activate_state(std::type_index index)
    {
      auto instance_it = state_map_.find(index);
      if (instance_it != state_map_.end())
      {
        auto guard = transition_guard();

        Transition transition;
        transition.type = Activate;
        transition.state_info.index = index;
        transition.state_info.state = instance_it->second.get();
        state_transitions_.push_back(transition);
      }
    }

    template <typename StateType>
    void StateMachine<StateType>::commit_state_transitions()
    {
      // Store the active state for later use
      StateType* previous_state = !state_stack_.empty() ? state_stack_.back().state : nullptr;

      // Go through all transitions
      for (const Transition& transition : state_transitions_)
      {
        const StateInfo& state_info = transition.state_info;

        // If there is an entry of the same type, remove it - if it's an activation,
        // it needs to go to the back and if not, it needs to be removed anyway.
        auto find_it = std::find_if(state_stack_.begin(), state_stack_.end(), MatchState(state_info.index));
        if (find_it != state_stack_.end())
        {
          state_stack_.erase(find_it);
        }

        if (transition.type == Destroy)
        {
          auto it = state_map_.find(state_info.index);
          if (it != state_map_.end())
          {
            if (it->second.get() == previous_state)
            {
              call_deactivation_function(previous_state);
              previous_state = nullptr;              
            }

            state_map_.erase(it);
          }            
        }

        // Finally, if it's an activation, append it to the back.
        else if (transition.type == Activate)
        {
          state_stack_.push_back(state_info);
        }
      }

      state_transitions_.clear();

      StateType* active_state = this->active_state();
      if (active_state != previous_state)
      {
        // The new state is different from the old state.
        // Let's call the callbacks.

        if (previous_state)
        {
          call_deactivation_function(previous_state);
        }

        if (active_state)
        {
          call_activation_function(active_state);
        }
      }
    }

    template <typename StateType>
    StateType* StateMachine<StateType>::active_state()
    {
      return !state_stack_.empty() ? state_stack_.back().state : nullptr;
    }

    template <typename StateType>
    const StateType* StateMachine<StateType>::active_state() const
    {
      return !state_stack_.empty() ? state_stack_.back().state : nullptr;
    }

    template <typename StateType>
    void StateMachine<StateType>::pop_state()
    {
      deactivate_state(state_stack_.back().index);
    }

    template <typename StateType>
    void StateMachine<StateType>::clear()
    {
      {
        auto guard = transition_guard();
        for (auto state_it = state_stack_.rbegin(); state_it != state_stack_.rend(); ++state_it)
        {
          deactivate_state(state_it->index);
        }
      }
    }

    template <typename StateType>
    StateMachine<StateType>::operator bool() const
    {
      return !state_stack_.empty();
    }

    template <typename StateType>
    typename StateMachine<StateType>::transition_guard_type StateMachine<StateType>::transition_guard()
    {
      return transition_guard_type(this);
    }

    // operator-> will invoke undefined behavior if there is no active state.
    template <typename StateType>
    StateType* StateMachine<StateType>::operator->()
    {
      return state_stack_.back().state;
    }

    template <typename StateType>
    const StateType* StateMachine<StateType>::operator->() const
    {
      return state_stack_.back().state;
    }

    namespace detail
    {
      // SFINAE at work: call activate(*state) if it exists, do nothing otherwise.
      template <typename StateType>
      void activation_helper(StateType* state, decltype(activate(std::declval<StateType&>()), int()) = 0)
      {
        activate(*state);
      }

      template <typename StateType>
      void activation_helper(StateType* state, void* = nullptr)
      {
      }

      // Ditto, but for deactivation.
      template <typename StateType>
      void deactivation_helper(StateType* state, decltype(deactivate(std::declval<StateType&>()), int()) = 0)
      {
        deactivate(*state);
      }

      template <typename StateType>
      void deactivation_helper(StateType* state, void* = nullptr)
      {
      }
    }

    template <typename StateType>
    void StateMachine<StateType>::call_activation_function(StateType* state) const
    {
      detail::activation_helper(state);
    }

    template <typename StateType>
    void StateMachine<StateType>::call_deactivation_function(StateType* state) const
    {
      detail::deactivation_helper(state);
    }


    namespace state_machine
    {
      template <typename StateType>
      TransitionGuard<StateType>::TransitionGuard(state_machine_type* state_machine)
        : state_machine_(state_machine)
      {
        ++state_machine->transition_guard_count_;
      }

      template <typename StateType>
      TransitionGuard<StateType>::TransitionGuard(const TransitionGuard& guard)
        : state_machine_(guard.state_machine_)
      {
        ++state_machine->transition_guard_count_;
      }

      template <typename StateType>
      TransitionGuard<StateType>& TransitionGuard<StateType>::operator=(const TransitionGuard& guard)
      {
        if (state_machine_ != guard.state_machine_ && 
          --state_machine_->transition_guard_count_ == 0)
        {
          state_machine_->commit_state_transitions();
        }

        state_machine_ = guard.state_machine_;
        ++state_machine_->transition_guard_count_;
      }

      template <typename StateType>
      TransitionGuard<StateType>::~TransitionGuard()
      {
        if (--state_machine_->transition_guard_count_ == 0)
        {
          state_machine_->commit_state_transitions();
        }
      }
    }

    template <typename StateType>
    StateMachine<StateType>::StateInfo::StateInfo(std::type_index index_, StateType* state_)
      : index(index_),
      state(state_)
    {
    }

    template <typename StateType>
    StateMachine<StateType>::MatchState::MatchState(std::type_index index)
      : index_(index)
    {
    }

    template <typename StateType>
    bool StateMachine<StateType>::MatchState::operator()(const StateInfo& entry) const
    {
      return entry.index == index_;
    }
  }
}

#endif