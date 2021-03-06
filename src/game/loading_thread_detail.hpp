/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "loading_thread.hpp"

namespace ts
{
  namespace game
  {
    template <typename FuncType>
    LoadingTaskModel<FuncType>::LoadingTaskModel(FuncType&& func)
      : func_(std::move(func))
    {}

    template <typename FuncType>
    void LoadingTaskModel<FuncType>::operator()()
    {
      try 
      {
        auto result = func_();

        promise_.set_value(std::move(result));
      }        

      catch (...)
      {
        promise_.set_exception(std::current_exception());
      }
    }

    template <typename FuncType>
    auto LoadingThread::async_task(FuncType&& func)
    {
      using func_type = std::remove_reference_t<FuncType>;
      using task_type = LoadingTaskModel<func_type>;

      auto task = std::make_unique<task_type>(std::move(func));
      auto future = task->promise_.get_future();

      std::unique_lock<std::mutex> lock(mutex_);
      task_list_.push_back(std::move(task));
      ++task_count_;

      cv_.notify_one();

      return future;
    }
  }
}
