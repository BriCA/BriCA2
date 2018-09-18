#ifndef __BRICA_THREAD_POOL_HPP__
#define __BRICA_THREAD_POOL_HPP__

#include "brica/resource_pool.hpp"

#include <asio.hpp>

#include <functional>
#include <memory>
#include <thread>
#include <vector>

namespace brica {

class ThreadPool : public CountedPool {
 public:
  ThreadPool(std::size_t size = 0)
      : work(std::make_shared<asio::io_service::work>(io_service)) {
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

  void add(std::function<void()> f) {
    io_service.post([f] { f(); });
  }

 private:
  asio::io_service io_service;
  std::shared_ptr<asio::io_service::work> work;
  std::vector<std::thread> threads;
};

}  // namespace brica

#endif  // __BRICA_THREAD_POOL_HPP__
