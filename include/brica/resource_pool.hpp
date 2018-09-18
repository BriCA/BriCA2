#ifndef __BRICA_RESOURCE_POOL_HPP__
#define __BRICA_RESOURCE_POOL_HPP__

#include <functional>

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

}  // namespace brica

#endif  // __BRICA_RESOURCE_POOL_HPP__
