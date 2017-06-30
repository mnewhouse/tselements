/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "editor_action_history.hpp"

namespace ts
{
  namespace editor
  {
    ActionHistory::ActionHistory(std::size_t capacity)
      : action_capacity_(capacity)
    {
    }

    void ActionHistory::push_action(std::string description, function_type action, function_type undo_action, bool call)
    {
      if (call)
      {
        action();
      }

      ++current_action_index_;
      action_stack_.resize(current_action_index_);

      auto& back = action_stack_.back();
      back.description = std::move(description);
      back.func = std::move(action);
      back.undo_func = std::move(undo_action);
            
      while (current_action_index_ > action_capacity_)
      {
        action_stack_.pop_front();
      }
    }

    bool ActionHistory::can_undo() const
    {
      return current_action_index_ != 0;
    }

    bool ActionHistory::can_redo() const
    {
      return current_action_index_ < action_stack_.size();
    }

    bool ActionHistory::can_undo(std::size_t count) const
    {
      return current_action_index_ >= count;
    }

    bool ActionHistory::can_redo(std::size_t count) const
    {
      return action_stack_.size() >= current_action_index_ + count;
    }

    void ActionHistory::undo()
    {
      if (can_undo())
      {
        auto new_index = current_action_index_ - 1;

        action_stack_[new_index].undo_func();
        current_action_index_ = new_index;       
      }
    }

    void ActionHistory::redo()
    {
      if (can_redo())
      {
        action_stack_[current_action_index_].func();

        ++current_action_index_;
      }
    }
    
    void ActionHistory::undo(std::size_t count)
    {
      while (count-- && can_undo())
      {
        undo();
      }
    }

    void ActionHistory::redo(std::size_t count)
    {
      while (count-- && can_redo())
      {
        redo();
      }
    }

    std::size_t ActionHistory::capacity() const
    {
      return action_capacity_;
    }

    void ActionHistory::set_capacity(std::size_t cap)
    {
      action_capacity_ = cap;
      if (current_action_index_ > action_capacity_)
      {
        current_action_index_ = action_capacity_;
      }

      if (action_capacity_ < action_stack_.size())
      {
        auto count = action_stack_.size() - action_capacity_;
        action_stack_.erase(action_stack_.begin(), action_stack_.begin() + count);
      }
    }

    std::size_t ActionHistory::stack_size() const
    {
      return action_stack_.size();
    }

    std::size_t ActionHistory::current_index() const
    {
      return current_action_index_;
    }

    const std::string& ActionHistory::action_description(std::size_t index) const
    {
      return action_stack_[index].description;
    }
  }
}