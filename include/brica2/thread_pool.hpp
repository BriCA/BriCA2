#ifndef __BRICA2_THREAD_POOL_HPP__
#define __BRICA2_THREAD_POOL_HPP__

#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>

NAMESPACE_BEGIN(BRICA2_NAMESPACE)

class thread_pool {
 public:
  thread_pool(std::size_t size) : stop(false) {
    for (std::size_t i = 0; i < size; ++i) {
      workers.emplace_back([this] { spawn(); });
    }
  }

  virtual ~thread_pool() {
    if (!stop) join();
  }

  void post(std::function<void()> f) {
    std::unique_lock<std::mutex> lock(mutex);
    tasks.push(f);
    condition.notify_one();
  }

  void join() {
    {
      std::unique_lock<std::mutex> lock(mutex);
      stop = true;
    }

    condition.notify_all();

    for (std::size_t i = 0; i < workers.size(); ++i) {
      workers[i].join();
    }
  }

 private:
  void spawn() {
    std::function<void()> task;
    while (!stop) {
      std::unique_lock<std::mutex> lock(mutex);
      condition.wait(
          lock, [this] { return (!tasks.empty()) || (tasks.empty() && stop); });
      if (!tasks.empty()) {
        task = std::move(tasks.front());
        tasks.pop();
        task();
      }
    }
  }

  std::vector<std::thread> workers;
  std::queue<std::function<void()>> tasks;

  std::mutex mutex;
  std::condition_variable condition;
  bool stop;
};

inline void dispatch(thread_pool& pool, std::function<void()> f) {
  pool.post(f);
}

NAMESPACE_END(BRICA2_NAMESPACE)

#endif  // __BRICA2_THREAD_POOL_HPP__
