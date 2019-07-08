#ifndef __BRICA2_MPI_INSTANCE_HPP__
#define __BRICA2_MPI_INSTANCE_HPP__

#include "mpi.h"

NAMESPACE_BEGIN(BRICA2_NAMESPACE)
NAMESPACE_BEGIN(mpi)

struct scoped_instance {
  scoped_instance(int& argc, char* argv[]) { MPI_Init(&argc, &argv); }
  virtual ~scoped_instance() { MPI_Finalize(); }
};

NAMESPACE_END(mpi)
NAMESPACE_END(BRICA2_NAMESPACE)

#endif  // __BRICA2_MPI_INSTANCE_HPP__

