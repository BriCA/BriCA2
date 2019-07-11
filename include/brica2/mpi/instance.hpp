#ifndef __BRICA2_MPI_INSTANCE_HPP__
#define __BRICA2_MPI_INSTANCE_HPP__

#include "mpi.h"

namespace brica2 {
namespace mpi {

struct scoped_instance {
  scoped_instance(int& argc, char* argv[]) { MPI_Init(&argc, &argv); }
  virtual ~scoped_instance() { MPI_Finalize(); }
};

}  // namespace mpi
}  // namespace brica2

#endif  // __BRICA2_MPI_INSTANCE_HPP__
