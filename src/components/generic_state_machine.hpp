/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <vector>
#include <typeindex>
#include <cstddef>
#include <unordered_map>
#include <memory>

namespace ts
{
  namespace components
  {
    template <typename StateType>
    class StateMachine;

    namespace state_machine
    {
      struct DeferredTag {};

      static const DeferredTag deferred;

      template <typename StateType>
      struct TransitionGuard
      {
        using state_machine_type = StateMachine<StateType>;

        ~TransitionGuard();

        TransitionGuard(const TransitionGuard&);
        TransitionGuard& operator=(const TransitionGuard&);

      private:
        TransitionGuard(state_machine_type* state_machine);
        friend state_machine_type;

        state_machine_type* state_machine_;
      };
    }

    // class template StateMachine
    // Allows for construction and management of game states.
    // Concrete types must derive from the StateType template argument,
    // and every concrete type is limited to one instance.
    //
    // Active instances are stored in a stack, and the instance on the stack's top
    // can be acted on through operator->.
    // When a state is activated, the free function activate(*StatePtr) is called
    // if it exists, using ADL as needed. Similarly, deactivate(*StatePtr) is called
    // when a state is deactivated.
    template <typename StateType>
    class StateMachine
    {
    public:
      using transition_guard_type = state_machine::TransitionGuard<StateType>;

      StateMachine() = default;

      // The destructor must destroy the active states in reverse order, then destroys the rest.
      ~StateMachine();

      StateMachine(const StateMachine&) = delete;
      StateMachine& operator=(const StateMachine&) = delete;

      // Constructs a new instance of a specified state type with the given constructor arguments.
      // If an instance of the type already exists, nothing happens.
      // This function activates the new state by pushing it onto the state stack,
      // then returns a pointer to it.
      template <typename ConcreteType, typename... Args>
      ConcreteType* create_state(Args&&... args);

      // Constructs a new instance of a specified state type, but does not activate it.
      // Returns a pointer to the new instance.
      template <typename ConcreteType, typename... Args>
      ConcreteType* create_state(state_machine::DeferredTag, Args&&... args);

      // Destroy a state given its concrete type.
      template <typename ConcreteType>
      void destroy_state();

      // Switch to a state given its concrete type.
      // Will do nothing if there is no instance of said type.
      template <typename ConcreteType>
      void activate_state();

      // Clear the state stack.
      void clear();

      void pop_state();

      // operator-> returns a pointer to the active state if there is one,
      // but will invoke undefined behavior otherwise.
      StateType* operator->();
      const StateType* operator->() const;

      StateType* active_state();
      const StateType* active_state() const;

      explicit operator bool() const;

      transition_guard_type transition_guard();

    private:
      struct StateInfo
      {
        StateInfo(std::type_index index_ = typeid(void), StateType* state_ = nullptr);

        std::type_index index;
        StateType* state;
      };

      template <typename ConcreteType>
      void deactivate_state();

      void activate_state(std::type_index index);
      void deactivate_state(std::type_index index);

      void call_activation_function(StateType* state_ptr) const;
      void call_deactivation_function(StateType* state_ptr) const;

      void commit_state_transitions();

      std::unordered_map<std::type_index, std::unique_ptr<StateType>> state_map_;
      std::vector<StateInfo> state_stack_;

      // Helper struct to find a stack entry with the given type index
      struct MatchState
      {
        MatchState(std::type_index index);

        bool operator()(const StateInfo& entry) const;

      private:
        std::type_index index_;
      };

      enum TransitionType
      {
        Activate,
        Deactivate,
        Destroy
      };

      struct Transition
      {
        StateInfo state_info;
        TransitionType type;
      };

      friend transition_guard_type;
      std::vector<Transition> state_transitions_;
      std::size_t transition_guard_count_ = 0;
    };
  }
}

#include "generic_state_machine.inl"
