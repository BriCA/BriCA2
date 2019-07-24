#ifndef __BRICA2_EXECUTOR_OMP_HPP__
#define __BRICA2_EXECUTOR_OMP_HPP__

#include "brica2/executor.hpp"

#include <omp.h>

namespace brica2 {

class omp : public executor_type {
 public:
  virtual void post(std::function<void()> f) override { fs.push_back(f); }

  virtual void sync() override {
    std::size_t i;
#pragma omp parallel for
    for (i = 0; i < fs.size(); ++i) {
      fs[i]();
    }
    fs.clear();
  }

 private:
  std::vector<std::function<void()>> fs;
};

}  // namespace brica2

#endif  // __BRICA2_EXECUTOR_OMP_HPP__
