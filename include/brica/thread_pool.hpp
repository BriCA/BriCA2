#ifndef __BRICA_THREAD_POOL_HPP__
#define __BRICA_THREAD_POOL_HPP__

#include <asio.hpp>

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace brica {

class ThreadPool {
 public:
  ThreadPool(std::size_t size)
      : work(std::make_shared<asio::io_service::work>(io_service)),
        count(0),
        total(0) {
    if (size == 0) {
      size = std::thread::hardware_concurrency();
    }

    for (std::size_t i = 0; i < size; ++i) {
      threads.emplace_back([this] { io_service.run(); });
    }
  }

  ~ThreadPool() {
    work.reset();
    for (std::size_t i = 0; i < threads.size(); ++i) {
      threads[i].join();
    }
  }

  void enqueue(std::function<void()> f) {
    total++;

    io_service.post([this, f] {
      f();
      count++;
      cond.notify_all();
    });
  }

  void wait() {
    if (count == total) return;

    std::unique_lock<std::mutex> lock(mutex);
    cond.wait(lock, [this] { return count == total; });
    count = 0;
    total = 0;
  }

 private:
  asio::io_service io_service;
  std::shared_ptr<asio::io_service::work> work;

  std::vector<std::thread> threads;

  std::atomic<std::size_t> count;
  std::atomic<std::size_t> total;

  std::mutex mutex;
  std::condition_variable cond;
};

}  // namespace brica

#endif  // __BRICA_THREAD_POOL_HPP__
