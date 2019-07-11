#ifndef __BRICA2_MPI_EXECUTOR_HPP__
#define __BRICA2_MPI_EXECUTOR_HPP__

#include "mpi.h"

namespace brica2 {
namespace mpi {

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

}  // namespace mpi
}  // namespace brica2

#endif  // __BRICA2_MPI_EXECUTOR_HPP__
