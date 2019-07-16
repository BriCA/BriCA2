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

using thread_count_t = decltype(std::thread::hardware_concurrency());

inline auto default_concurrency(thread_count_t n) -> decltype(auto) {
  if (n == 0) {
    auto value = std::thread::hardware_concurrency();
    return value == 0 ? thread_count_t(1) : value;
  }
  return n;
}

class parallel : public executor_type {
 public:
  parallel(thread_count_t n = 0)
      : pool(default_concurrency(n)), count(0), total(0) {}

  virtual ~parallel() { pool.join(); }

  virtual void post(std::function<void()> f) override {
    ++total;
    dispatch(pool, [=] {
      f();
      ++count;
      std::unique_lock<std::mutex> lock{mutex};
      lock.unlock();
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
