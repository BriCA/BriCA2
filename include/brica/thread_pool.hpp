/******************************************************************************
 *
 * brica/ThreadPool.hpp
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *****************************************************************************/

#ifndef __BRICA_THREAD_POOL_HPP__
#define __BRICA_THREAD_POOL_HPP__

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace brica {

class ThreadPool {
 public:
  ThreadPool(std::size_t size) : stop(false) {
    for (std::size_t i = 0; i < size; ++i) {
      workers.emplace_back([this] { spawn(); });
    }
  }

  virtual ~ThreadPool() {
    if (!stop) join();
  }

  void post(std::function<void()>& f) {
    {
      std::lock_guard<std::mutex> lock{mutex};
      tasks.push(f);
    }

    condition.notify_one();
  }

  void join() {
    {
      std::lock_guard<std::mutex> lock{mutex};
      stop = true;
    }

    condition.notify_all();

    for (std::size_t i = 0; i < workers.size(); ++i) {
      workers[i].join();
    }
  }

  std::size_t size() const { return workers.size(); }

 private:
  void spawn() {
    for (;;) {
      std::function<void()> task;
      {
        std::unique_lock<std::mutex> lock{mutex};
        condition.wait(lock, [this] { return !tasks.empty() || stop; });
        if (stop && tasks.empty()) return;
        task = std::move(tasks.front());
        tasks.pop();
      }
      task();
    }
  }

  std::vector<std::thread> workers;
  std::queue<std::function<void()>> tasks;

  std::mutex mutex;
  std::condition_variable condition;
  bool stop;
};

inline void dispatch(ThreadPool& pool, std::function<void()> f) {
  pool.post(f);
}

}  // namespace brica

#endif  // __BRICA_THREAD_POOL_HPP__
