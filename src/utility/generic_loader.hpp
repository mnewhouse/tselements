/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <string>
#include <atomic>
#include <future>
#include <functional>

namespace ts
{
  namespace utility
  {
    template <typename StateType>
    class LoadingInterface
    {
    public:
      explicit LoadingInterface(double max_progress = 1.0)
        : max_progress_(max_progress)
      {}

      bool is_loading() const { return is_loading_; }
      double progress() const { return progress_; }
      double max_progress() const { return max_progress_; }
      StateType loading_state() const { return loading_state_; }

      void set_loading_state(StateType state) { loading_state_ = state; }
      void set_progress(double progress) { progress_ = progress; }
      void set_max_progress(double max_progress) { max_progress_ = max_progress; }

    protected:
      void set_loading(bool loading) { is_loading_ = loading; }

    private:
      std::atomic<bool> is_loading_ = false;
      std::atomic<double> progress_ = 0.0;
      std::atomic<double> max_progress_ = 1.0;
      std::atomic<StateType> loading_state_ = {};
    };

    // This class template is a really simple generic base class that provides
    // functionality to asynchronously perform a loading task.
    template <typename StateType, typename ResultType>
    class GenericLoader
      : public LoadingInterface<StateType>
    {
    public:
      explicit GenericLoader(StateType state = StateType(), double max_progress = 1.0);

      ResultType get_result();
      bool is_ready() const;

    protected:
      ~GenericLoader() = default;

      template <typename F, typename... Args>
      void async_load(F&& func, Args&&... args);      

    private:
      std::future<ResultType> future_;
    };


    template <typename StateType, typename ResultType>
    GenericLoader<StateType, ResultType>::GenericLoader(StateType state, double max_progress)
      : LoadingInterface<StateType>(max_progress)
    {
    }

    template <typename StateType, typename ResultType>
    template <typename F, typename... Args>
    void GenericLoader<StateType, ResultType>::async_load(F&& function, Args&&... args)
    {
      this->set_loading(true);
      this->set_progress(0.0);

      future_ = std::async(std::launch::async, std::forward<F>(function), std::forward<Args>(args)...);
    }

    template <typename StateType, typename ResultType>
    bool GenericLoader<StateType, ResultType>::is_ready() const
    {
      return this->is_loading() && future_.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
    }

    template <typename StateType, typename ResultType>
    ResultType GenericLoader<StateType, ResultType>::get_result()
    {
      try
      {
        this->set_loading(false);
        return future_.get();
      }

      catch (...)
      {
        future_ = {};
        throw;
      }
    }
  }
}
