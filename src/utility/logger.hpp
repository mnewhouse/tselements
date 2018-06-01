/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <vector>
#include <mutex>
#include <atomic>
#include <thread>
#include <fstream>

#include <boost/interprocess/streams/vectorstream.hpp>

namespace ts
{
  namespace logger
  {
    namespace impl
    {
      struct flush_helper
      {
        template <typename DispatcherType>
        void operator()(DispatcherType& dispatcher) const
        {
          flush_impl(dispatcher, 0);
        }

      private:
        template <typename DispatcherType>
        void flush_impl(DispatcherType& dispatcher, ...) const
        {
        }

        template <typename DispatcherType>
        std::enable_if_t<true, decltype(std::declval<DispatcherType>().flush(), (void)0)>
          flush_impl(DispatcherType& dispatcher, int) const
        {
          dispatcher.flush();
        }
      };
    }

    struct EmptyConfig
    {
    };

    struct DefaultFilter
    {
      template <typename Config>
      void operator()(const Config& config);
    };

    class LogFileDispatcher;

    struct DefaultLoggerTraits
    {
      using config_type = EmptyConfig;
      using dispatcher_type = LogFileDispatcher;
    };

    template <typename Tag>
    struct LoggerTraits
    {
      using config_type = typename Tag::config_type;
      using dispatcher_type = typename Tag::config_type;
    };

    // ScopedLogger initializes a logger singleton on construction, and shuts it down
    // on destruction, RAII-style.
    template <typename Tag>
    struct ScopedLogger
    {
    public:
      template <typename... Args>
      ScopedLogger(Args&&... args);

      ~ScopedLogger();

      ScopedLogger(const ScopedLogger&) = delete;
      ScopedLogger<Tag>& operator=(const ScopedLogger&) = delete;
    };

    struct flush_t {};
    static const flush_t flush;

    struct endl_t {};
    static const endl_t endl;

    // Logger class template. It is a singleton, and for internal use only.
    // The public interface is in the LoggerFront class template.
    template <typename Tag>
    class Logger
    {
    public:
      using dispatcher_type = typename LoggerTraits<Tag>::dispatcher_type;
      using config_type = typename LoggerTraits<Tag>::config_type;

      template <typename... Args>
      explicit Logger(config_type config, Args&&... args)
        : dispatcher_(std::forward<Args>(args)...),
          config_(config)
      {
      }

      const config_type& config() const;

      struct LockReleaser;
      using Lock = std::unique_ptr<Logger<Tag>, LockReleaser>;
      Lock acquire_lock();
      void release_lock();

      template <typename T>
      Logger<Tag>& operator<<(const T& value);

      Logger<Tag>& operator<<(flush_t);
      Logger<Tag>& operator<<(endl_t);

      void dispatch();

    private:
      dispatcher_type dispatcher_;
      config_type config_;

      boost::interprocess::basic_vectorstream<std::vector<char>> stream_;
      std::string buffer_;
      std::mutex mutex_;
    };

    template <typename Tag>
    struct Logger<Tag>::LockReleaser
    {
      void operator()(Logger<Tag>* logger) const
      {
        logger->release_lock();
      }
    };

    template <typename Tag>
    class LoggerFront
    {
    public:
      LoggerFront();
      ~LoggerFront();

      template <typename T>
      LoggerFront& operator<<(const T& value);

      using config_type = typename LoggerTraits<Tag>::config_type;
      const config_type& config() const;

    private:
      friend ScopedLogger<Tag>;

      template <typename... Args>
      static void initialize(Args&&... args);
      static void shutdown();

      static std::unique_ptr<Logger<Tag>> logger_instance_;
      typename Logger<Tag>::Lock lock_;
    };

    // Asynchronous log file dispatcher.
    class AsyncLogFileDispatcher
    {
    public:
      AsyncLogFileDispatcher(const std::string& out_file);
      ~AsyncLogFileDispatcher();

      template <typename InputIt>
      void dispatch(InputIt it, InputIt end);

    private:
      void worker_thread(const std::string& file_name);

      std::vector<char> out_buffer_;
      std::atomic<bool> is_running_;
      std::thread worker_thread_;
      std::mutex mutex_;
    };

    class LogFileDispatcher
    {
    public:
      LogFileDispatcher(const std::string& out_file);

      template <typename InputIt>
      void dispatch(InputIt it, InputIt end);

      void flush();

    private:
      std::ofstream stream_;
    };
  }
}

// Singleton instance
template <typename Tag>
std::unique_ptr<ts::logger::Logger<Tag>> ts::logger::LoggerFront<Tag>::logger_instance_;

template <typename Tag>
ts::logger::LoggerFront<Tag>::LoggerFront()
    : lock_(logger_instance_ ? logger_instance_->acquire_lock() : nullptr)
{
}

template <typename Tag>
ts::logger::LoggerFront<Tag>::~LoggerFront()
{
    if (logger_instance_)
    {
        logger_instance_->dispatch();
    }
}

template <typename Tag>
const typename ts::logger::LoggerFront<Tag>::config_type& ts::logger::LoggerFront<Tag>::config() const
{
    return logger_instance_->config();
}

template <typename Tag>
template <typename... Args>
void ts::logger::LoggerFront<Tag>::initialize(Args&&... args)
{
    logger_instance_ = std::make_unique<Logger<Tag>>(std::forward<Args>(args)...);
}

template <typename Tag>
void ts::logger::LoggerFront<Tag>::shutdown()
{
    logger_instance_ = nullptr;
}

template <typename Tag>
template <typename T>
ts::logger::LoggerFront<Tag>& ts::logger::LoggerFront<Tag>::operator<<(const T& value)
{
    *logger_instance_ << value;
    return *this;
}

template <typename Tag>
const typename ts::logger::Logger<Tag>::config_type& ts::logger::Logger<Tag>::config() const
{
    return config_;
}

// Locking functions.
template <typename Tag>
typename ts::logger::Logger<Tag>::Lock ts::logger::Logger<Tag>::acquire_lock()
{
    mutex_.lock();
    return Lock(this);
}

template <typename Tag>
void ts::logger::Logger<Tag>::release_lock()
{
    mutex_.unlock();
}

template <typename Tag>
template <typename T>
ts::logger::Logger<Tag>& ts::logger::Logger<Tag>::operator<<(const T& value)
{
    stream_ << value;
    return *this;
}

template <typename Tag>
ts::logger::Logger<Tag>& ts::logger::Logger<Tag>::operator<<(flush_t)
{
    dispatch();

    impl::flush_helper helper;
    helper(dispatcher_);
    return *this;
}

template <typename Tag>
ts::logger::Logger<Tag>& ts::logger::Logger<Tag>::operator<<(endl_t)
{
    return *this << "\n" << flush;
}

template <typename Tag>
void ts::logger::Logger<Tag>::dispatch()
{
    const auto& data = stream_.vector();
    dispatcher_.dispatch(data.begin(), data.end());
    stream_.clear();
}

template <typename Tag>
template <typename... Args>
ts::logger::ScopedLogger<Tag>::ScopedLogger(Args&&... args)
{
    LoggerFront<Tag>::initialize(std::forward<Args>(args)...);
}

template <typename Tag>
ts::logger::ScopedLogger<Tag>::~ScopedLogger()
{
    LoggerFront<Tag>::shutdown();
}


// Dispatch functions
template <typename InputIt>
void ts::logger::AsyncLogFileDispatcher::dispatch(InputIt it, InputIt end)
{
    std::unique_lock<std::mutex> lock(mutex_);
    out_buffer_.insert(out_buffer_.end(), it, end);
}

template <typename InputIt>
void ts::logger::LogFileDispatcher::dispatch(InputIt it, InputIt end)
{
  if (stream_)
  {
    std::copy(it, end, std::ostreambuf_iterator<char>(stream_));
  }
}
