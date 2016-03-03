/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef LOADING_THREAD_HPP_5981891822
#define LOADING_THREAD_HPP_5981891822

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
    namespace detail
    {
      struct TaskBase
      {
        virtual ~TaskBase() = default;

        virtual void operator()() = 0;
      };

      template <typename FuncType>
      struct GenericTask
        : TaskBase
      {
        using result_type = decltype(std::declval<FuncType>()());

        explicit GenericTask(FuncType&& func);

        virtual void operator()() override;

        std::promise<result_type> promise_;
        FuncType func_;
      };
    }

    class LoadingThread
    {
    public:
      explicit LoadingThread();
      ~LoadingThread();

      LoadingThread(LoadingThread&&) = default;
      LoadingThread& operator=(LoadingThread&&) = default;

      template <typename FuncType>
      auto async_task(FuncType&& func);

    private:
      void worker_function();

      std::thread worker_thread_;
      std::mutex mutex_;
      std::condition_variable cv_;
      std::atomic<bool> is_finished_ = false;
      std::atomic<std::uint32_t> task_count_ = 0;

      std::deque<std::unique_ptr<detail::TaskBase>> task_list_;
    };
  }
}

#include "loading_thread_detail.hpp"

#endif