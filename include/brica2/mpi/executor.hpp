#ifndef __BRICA2_MPI_EXECUTOR_HPP__
#define __BRICA2_MPI_EXECUTOR_HPP__

#include "mpi.h"

#include "brica2/executor/parallel.hpp"

namespace brica2 {
namespace mpi {

template <class Executor> class executor : public Executor {
 public:
  explicit executor(MPI_Comm c = MPI_COMM_WORLD) : comm(c) {}
  virtual ~executor() {}

  virtual void post(std::function<void()> f) override { Executor::post(f); }
  virtual void sync() override {
    Executor::sync();
    MPI_Barrier(comm);
  }

 private:
  MPI_Comm comm;
};

template <> class executor<parallel> : public parallel {
 public:
  explicit executor(thread_count_t n = 0, MPI_Comm c = MPI_COMM_WORLD)
      : parallel(n), comm(c) {}

  virtual void post(std::function<void()> f) override { parallel::post(f); }
  virtual void sync() override {
    parallel::sync();
    MPI_Barrier(comm);
  }

 private:
  MPI_Comm comm;
};

}  // namespace mpi
}  // namespace brica2

#endif  // __BRICA2_MPI_EXECUTOR_HPP__
