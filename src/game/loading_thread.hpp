/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <thread>
#include <future>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <deque>

namespace ts
{
  namespace game
  {
    struct LoadingTaskBase
    {
      virtual ~LoadingTaskBase() = default;

      virtual void operator()() = 0;
    };

    template <typename FuncType>
    struct LoadingTaskModel
      : LoadingTaskBase
    {
      using result_type = decltype(std::declval<FuncType>()());

      explicit LoadingTaskModel(FuncType&& func);

      virtual void operator()() override;
      
      std::promise<result_type> promise_;
      FuncType func_;
    };

    // The LoadingThread wraps a single thread that can be used to execute various tasks
    // asynchronously. This worker thread has an internal OpenGL context that can be
    // shared with other threads' GL contexts.
    class LoadingThread
    {
    public:
      explicit LoadingThread();
      ~LoadingThread();

      LoadingThread(LoadingThread&&) = default;
      LoadingThread& operator=(LoadingThread&&) = default;

      // Execute a task asynchronously. Returns a std::future object.
      template <typename FuncType>
      auto async_task(FuncType&& func);

    private:
      void worker_function();

      std::thread worker_thread_;
      std::mutex mutex_;
      std::condition_variable cv_;
      std::atomic<bool> is_finished_ = false;
      std::atomic<std::uint32_t> task_count_ = 0;

      std::deque<std::unique_ptr<LoadingTaskBase>> task_list_;
    };
  }
}

#include "loading_thread_detail.hpp"
