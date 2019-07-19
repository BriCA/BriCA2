#ifndef __BRICA2_MPI_INSTANCE_HPP__
#define __BRICA2_MPI_INSTANCE_HPP__

#include <atomic>
#include <mutex>

#include "mpi.h"

namespace brica2 {
namespace mpi {

class scoped_instance {
 public:
  scoped_instance(int& argc, char* argv[]) {
    std::lock_guard<std::mutex> lock{mutex};
    if (!initialized) MPI_Init(&argc, &argv);
    initialized = true;
  }

  virtual ~scoped_instance() {
    std::lock_guard<std::mutex> lock{mutex};
    if (initialized) MPI_Finalize();
    initialized = false;
  }

 private:
  static std::atomic_bool initialized;
  std::mutex mutex;
};

std::atomic_bool scoped_instance::initialized{false};

}  // namespace mpi
}  // namespace brica2

#endif  // __BRICA2_MPI_INSTANCE_HPP__
