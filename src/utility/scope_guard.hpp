/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <boost/optional.hpp>

#include <utility>

namespace ts
{
  template <typename NullaryFunction>
  class ScopeGuard
  {
  public:
    struct construct_tag {};

    template <typename F>
    ScopeGuard(construct_tag, F&& func)
      : func_(std::forward<F>(func))
    {
    }

    ~ScopeGuard()
    {
      if (func_) (*func_)();
    }

    ScopeGuard(std::nullptr_t)
      : func_(boost::none)
    {
    }

    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;

    ScopeGuard(ScopeGuard&& other)
      : func_(std::move(other.func_))
    {
      other.func_ = boost::none;
    }

    ScopeGuard& operator=(ScopeGuard&& other)
    {
      if (func_) (*func_)();

      func_ = std::move(other.func_);
      other.func_ = boost::none;
      return *this;
    }

    void clear()
    {
      if (func_) (*func_)();

      func_ = boost::none;
    }

    explicit operator bool() const
    {
      return func_.is_initialized();
    }

  private:
    boost::optional<NullaryFunction> func_;
  };

  template <typename NullaryFunction>
  auto make_scope_guard(NullaryFunction&& f)
  {
    using guard_type = ScopeGuard<std::remove_reference_t<NullaryFunction>>;
    return guard_type(typename guard_type::construct_tag{}, f);
  }
}