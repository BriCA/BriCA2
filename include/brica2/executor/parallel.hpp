#ifndef __BRICA2_EXECUTOR_PARALLEL_HPP__
#define __BRICA2_EXECUTOR_PARALLEL_HPP__

#include "brica2/macros.h"
#include "brica2/executor.hpp"
#include "brica2/thread_pool.hpp"

#include <functional>
#include <vector>
#include <atomic>
#include <mutex>
#include <condition_variable>

NAMESPACE_BEGIN(BRICA2_NAMESPACE)

inline auto default_concurrency() -> decltype(auto) {
  auto value = std::thread::hardware_concurrency();
  return value == 0 ? 1 : value;
}

class parallel : public executor_type {
 public:
  parallel(std::size_t n = default_concurrency())
      : pool(n), count(0), total(0) {}

  virtual ~parallel() { pool.join(); }

  virtual void post_impl(std::vector<std::function<void(void)>>& fs) override {
    total += fs.size();
    std::for_each(fs.begin(), fs.end(), [&](auto& f) {
      dispatch(pool, [&]() {
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
  thread_pool pool;
  std::atomic<std::size_t> count;
  std::atomic<std::size_t> total;
  std::mutex mutex;
  std::condition_variable condition;
};

NAMESPACE_END(BRICA2_NAMESPACE)

#endif  // __BRICA2_EXECUTOR_PARALLEL_HPP__
