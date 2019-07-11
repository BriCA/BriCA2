#ifndef __BRICA2_EXECUTOR_HPP__
#define __BRICA2_EXECUTOR_HPP__

#include <functional>

namespace brica2 {

class executor_type {
 public:
  virtual void post(std::function<void()> f) = 0;
  virtual void sync() = 0;
};

}  // namespace brica2

#endif  // __BRICA2_EXECUTOR_HPP__
