/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef LOADING_THREAD_DETAIL_HPP_1189182491824
#define LOADING_THREAD_DETAIL_HPP_1189182491824

#include "loading_thread.hpp"

namespace ts
{
  namespace game
  {
    namespace detail
    {
      template <typename FuncType>
      GenericTask<FuncType>::GenericTask(FuncType&& func)
        : func_(std::move(func))
      {}

      template <typename FuncType>
      void GenericTask<FuncType>::operator()()
      {
        try {
          auto result = func_();

          glFinish();

          promise_.set_value(std::move(result));
        }        

        catch (...)
        {
          promise_.set_exception(std::current_exception());
        }
      }
    }

    template <typename FuncType>
    auto LoadingThread::async_task(FuncType&& func)
    {
      using func_type = std::remove_reference_t<FuncType>;
      using task_type = detail::GenericTask<func_type>;

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

#endif