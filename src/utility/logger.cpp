/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "logger.hpp"

ts::logger::LogFileDispatcher::LogFileDispatcher(const std::string& output_file)
    : stream_(output_file, std::ofstream::out)
{
}

void ts::logger::LogFileDispatcher::flush()
{
    stream_.flush();
}

ts::logger::AsyncLogFileDispatcher::AsyncLogFileDispatcher(const std::string& output_file)
    : worker_thread_([this, output_file]() { worker_thread(output_file); }),
      is_running_(true)
{
}

void ts::logger::AsyncLogFileDispatcher::worker_thread(const std::string& output_file)
{
    std::ofstream file_stream(output_file, std::ofstream::out);

    while (file_stream && is_running_)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));

        {
            std::unique_lock<std::mutex> lock(mutex_);

            // Write all the data in the buffer to the stream.
            file_stream.write(out_buffer_.data(), out_buffer_.size());

            // Also clear the buffer.
            out_buffer_.clear();
        }

        // And flush the stream, we don't need the lock for this anymore.
        file_stream.flush();
    }

    while (is_running_)
    {
        // The stream is broken, so just keep clearing the buffer so that it doesn't get too big.
        std::this_thread::sleep_for(std::chrono::milliseconds(20));

        {
            std::unique_lock<std::mutex> lock(mutex_);
            out_buffer_.clear();
        }       
    }

    is_running_ = false;
}

ts::logger::AsyncLogFileDispatcher::~AsyncLogFileDispatcher()
{
    is_running_ = false;
    if (worker_thread_.joinable())
    {
        worker_thread_.join();
    }   
}