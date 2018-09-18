#ifndef __BRICA_RESOURCE_POOL_HPP__
#define __BRICA_RESOURCE_POOL_HPP__

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>

namespace brica {

class ResourcePool {
 public:
  virtual ~ResourcePool() {}
  virtual void enqueue(std::function<void()>) = 0;
  virtual void wait() = 0;
};

class DefaultPool : public ResourcePool {
 public:
  static DefaultPool& singleton() {
    static DefaultPool pool;
    return pool;
  }
  void enqueue(std::function<void()> f) { f(); }
  void wait() {}
};

class CountedPool : public ResourcePool {
 public:
  CountedPool() : count(0), total(0) {}

  virtual ~CountedPool() {}

  virtual void add(std::function<void()>) = 0;

  void enqueue(std::function<void()> f) {
    total++;
    add([this, f] {
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
  std::atomic<std::size_t> count;
  std::atomic<std::size_t> total;

  std::mutex mutex;
  std::condition_variable cond;
};

}  // namespace brica

#endif  // __BRICA_RESOURCE_POOL_HPP__
