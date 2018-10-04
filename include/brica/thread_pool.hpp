#ifndef __BRICA_THREAD_POOL_HPP__
#define __BRICA_THREAD_POOL_HPP__

#define ASIO_STANDALONE

#include "brica/resource_pool.hpp"

#include <functional>
#include <memory>
#include <thread>
#include <vector>

namespace brica {

class ThreadPool : public CountedPool {
 public:
  ThreadPool(std::size_t size = 0) {
    if (size == 0) {
      size = std::thread::hardware_concurrency();
    }

    for (std::size_t i = 0; i < size; ++i) {
      threads.emplace_back([this] {
        for (;;) {
          std::function<void()> task;

          {
            std::unique_lock<std::mutex> lock(mutex);
            cond.wait(lock);
            if (tasks.empty() && stop) {
              return;
            }
            task = std::move(tasks.front());
            tasks.pop();
          }

          task();
        }
      });
    }
  }

  ~ThreadPool() {
    {
      std::unique_lock<std::mutex> lock(mutex);
      stop = true;
    }
    cond.notify_all();
    for (std::size_t i = 0; i < threads.size(); ++i) {
      threads[i].join();
    }
  }

  void add(std::function<void()> f) {
    tasks.push([f] { f(); });
    cond.notify_one();
  }

 private:
  std::vector<std::thread> threads;
  std::queue<std::function<void()>> tasks;

  std::mutex mutex;
  std::condition_variable cond;
  bool stop;
};

}  // namespace brica

#endif  // __BRICA_THREAD_POOL_HPP__
