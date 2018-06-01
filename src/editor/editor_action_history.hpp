/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <functional>
#include <deque>

namespace ts
{
  namespace editor
  {
    // This class stores the actions the user has performed, enabling them to undo everything
    // up to a certain limit.
    class ActionHistory
    {
    public:
      using function_type = std::function<void()>;

      // Initialize the object with a given capacity. The capacity defines
      // how many actions can be stored in the stack.
      explicit ActionHistory(std::size_t action_capacity);

      // Perform an action, add it to the stack, and remove the oldest action from said
      // stack if we are at capacity. The "action" argument is invoked unless otherwise specified.
      // Calling "undo_action" must undo everything that "action" did.
      // The function objects are stored indefinitely, so prefer storing the needed state by value.
      void push_action(std::string description, function_type action, function_type undo_action, bool invoke = true);

      bool can_undo() const;
      bool can_redo() const;

      bool can_undo(std::size_t count) const;
      bool can_redo(std::size_t count) const;

      void undo(std::size_t count);
      void redo(std::size_t count);

      void undo();
      void redo();

      void set_capacity(std::size_t capacity);
      std::size_t capacity() const;

      std::size_t stack_size() const;
      std::size_t current_index() const;

      // index must be less than stack_size()
      const std::string& action_description(std::size_t index) const;

    private:
      struct Action
      {
        std::string description;
        function_type func;
        function_type undo_func;
      };

      std::deque<Action> action_stack_;
      std::size_t action_capacity_ = 64;
      std::size_t current_action_index_ = 0;
    };
  }
}