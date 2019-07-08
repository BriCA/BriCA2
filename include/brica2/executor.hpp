#ifndef __BRICA2_EXECUTOR_HPP__
#define __BRICA2_EXECUTOR_HPP__

#define ASIO_STANDALONE

#include "brica2/macros.h"

#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>

#include <asio.hpp>

#include <iostream>

NAMESPACE_BEGIN(BRICA2_NAMESPACE)

class executor_type {
 public:
  virtual void post_impl(std::vector<std::function<void(void)>>& fs) = 0;
  void post(std::vector<std::function<void(void)>>&& fs) { post_impl(fs); }
  void post(std::vector<std::function<void(void)>>& fs) { post_impl(fs); }
  virtual void sync() = 0;
};

struct serial : public executor_type {
  virtual void post_impl(std::vector<std::function<void(void)>>& fs) override {
    std::for_each(fs.begin(), fs.end(), [](auto& f) { f(); });
  }

  virtual void sync() override {}
};

inline auto default_concurrency() -> decltype(auto) {
  auto value = std::thread::hardware_concurrency();
  return value == 0 ? 1 : value;
}

class thread_parallel : public executor_type {
 public:
  thread_parallel(std::size_t n = default_concurrency())
      : pool(n), count(0), total(0) {}

  virtual ~thread_parallel() { pool.join(); }

  virtual void post_impl(std::vector<std::function<void(void)>>& fs) override {
    total += fs.size();
    std::for_each(fs.begin(), fs.end(), [&](auto& f) {
      asio::post(pool, [&]() {
        std::unique_lock<std::mutex> lock{mutex};
        f();
        ++count;
        condition.notify_all();
      });
    });
  }

  virtual void sync() override {
    std::unique_lock<std::mutex> lock{mutex};
    if (count != total) condition.wait(lock, [&]() { return count == total; });
    count = 0;
    total = 0;
  }

 private:
  asio::thread_pool pool;
  std::atomic<std::size_t> count;
  std::atomic<std::size_t> total;
  std::mutex mutex;
  std::condition_variable condition;
};

NAMESPACE_END(BRICA2_NAMESPACE)

#endif  // __BRICA2_EXECUTOR_HPP__
