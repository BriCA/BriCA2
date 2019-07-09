#ifndef __BRICA2_EXECUTOR_HPP__
#define __BRICA2_EXECUTOR_HPP__

#include "brica2/macros.h"

#include <functional>
#include <vector>

NAMESPACE_BEGIN(BRICA2_NAMESPACE)

class executor_type {
 public:
  virtual void post_impl(std::vector<std::function<void(void)>>& fs) = 0;
  void post(std::vector<std::function<void(void)>>&& fs) { post_impl(fs); }
  void post(std::vector<std::function<void(void)>>& fs) { post_impl(fs); }
  virtual void sync() = 0;
};

NAMESPACE_END(BRICA2_NAMESPACE)

#endif  // __BRICA2_EXECUTOR_HPP__
