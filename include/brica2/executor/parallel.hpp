#ifndef __BRICA2_EXECUTOR_PARALLEL_HPP__
#define __BRICA2_EXECUTOR_PARALLEL_HPP__

#include "brica2/executor.hpp"
#include "brica2/thread_pool.hpp"

#include <functional>
#include <vector>
#include <atomic>
#include <mutex>
#include <condition_variable>

namespace brica2 {

inline auto default_concurrency() -> decltype(auto) {
  auto value = std::thread::hardware_concurrency();
  return value == 0 ? 1 : value;
}

class parallel : public executor_type {
 public:
  parallel(std::size_t n = default_concurrency())
      : pool(n), count(0), total(0) {}

  virtual ~parallel() { pool.join(); }

  virtual void post(std::function<void()> f) override {
    ++total;
    dispatch(pool, [=] {
      f();
      ++count;
      std::lock_guard<std::mutex> lock{mutex};
      condition.notify_all();
    });
  }

  virtual void sync() override {
    std::unique_lock<std::mutex> lock{mutex};
    if (count != total) condition.wait(lock, [&]() { return count == total; });
    count = 0;
    total = 0;
  }

 private:
  thread_pool pool;
  std::atomic<std::size_t> count;
  std::atomic<std::size_t> total;
  std::mutex mutex;
  std::condition_variable condition;
};

}  // namespace brica2

#endif  // __BRICA2_EXECUTOR_PARALLEL_HPP__
