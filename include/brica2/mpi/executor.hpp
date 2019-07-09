#ifndef __BRICA2_MPI_EXECUTOR_HPP__
#define __BRICA2_MPI_EXECUTOR_HPP__

#include "mpi.h"

NAMESPACE_BEGIN(BRICA2_NAMESPACE)
NAMESPACE_BEGIN(mpi)

template <class Executor> class executor : public Executor {
 public:
  explicit executor(MPI_Comm c = MPI_COMM_WORLD) : comm(c) {}
  virtual ~executor() {}

  virtual void sync() override {
    Executor::sync();
    MPI_Barrier(comm);
  }

 private:
  MPI_Comm comm;
};

NAMESPACE_END(mpi)
NAMESPACE_END(BRICA2_NAMESPACE)

#endif  // __BRICA2_MPI_EXECUTOR_HPP__
