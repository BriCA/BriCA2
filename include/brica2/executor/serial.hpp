#ifndef __BRICA2_EXECUTOR_SERIAL_HPP__
#define __BRICA2_EXECUTOR_SERIAL_HPP__

#include "brica2/macros.h"
#include "brica2/executor.hpp"

#include <functional>
#include <vector>

NAMESPACE_BEGIN(BRICA2_NAMESPACE)

struct serial : public executor_type {
  virtual void post_impl(std::vector<std::function<void(void)>>& fs) override {
    std::for_each(fs.begin(), fs.end(), [](auto& f) { f(); });
  }

  virtual void sync() override {}
};

NAMESPACE_END(BRICA2_NAMESPACE)

#endif  // __BRICA2_EXECUTOR_SERIAL_HPP__
