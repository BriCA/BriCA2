#ifndef __BRICA2_EXECUTOR_SERIAL_HPP__
#define __BRICA2_EXECUTOR_SERIAL_HPP__

#include "brica2/executor.hpp"

#include <functional>
#include <vector>

namespace brica2 {

struct serial : public executor_type {
  virtual void post(std::function<void()> f) override { f(); }
  virtual void sync() override {}
};

}  // namespace brica2

#endif  // __BRICA2_EXECUTOR_SERIAL_HPP__
