/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"

#include "loading_thread.hpp"

#include "graphics/gl_context.hpp"

#include <GL/glew.h>

namespace ts
{
  namespace game
  {
    LoadingThread::LoadingThread()
      : worker_thread_([this]() { worker_function(); })
    {
    }

    LoadingThread::~LoadingThread()
    {
      is_finished_ = true;

      cv_.notify_one();
      worker_thread_.join();
    }

    void LoadingThread::worker_function()
    {
      auto context = graphics::create_gl_context();
      graphics::activate_gl_context(context);

      while (!is_finished_)
      {
        {
          std::unique_lock<std::mutex> lock(mutex_);
          cv_.wait(lock, [this]() { return is_finished_ || task_count_ != 0; });
        }

        while (task_count_ != 0)
        {
          auto pop_task = [this]()
          {
            std::unique_lock<std::mutex> lock(mutex_);
            auto task = std::move(task_list_.front());
            task_list_.pop_front();
            return task;
          };

          auto task = pop_task();
          (*task)();

          --task_count_;
        }
      }
    }
  }
}