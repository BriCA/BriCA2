#define CATCH_CONFIG_RUNNER
#include "brica/mpi.hpp"
#include "catch.hpp"

using namespace brica;

TEST_CASE("two component chain", "[proxy]") {
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  std::string key = "default";
  Buffer value({1, 2, 3});

  auto f_emit = [key, value](Dict& in, Dict& out) { out[key] = value; };
  auto f_null = [](Dict& in, Dict& out) {};

  mpi::Component emit(f_emit, 0);
  mpi::Component null(f_null, 1);

  emit.make_out_port(key);
  null.make_in_port(key);

  mpi::Proxy proxy(emit, key, null, key);

  REQUIRE(emit.get_output(key).empty());
  REQUIRE(null.get_input(key).empty());

  emit.execute();
  emit.expose();

  proxy.collect();
  null.collect();

  MPI_Barrier(MPI_COMM_WORLD);

  if (rank == 0) REQUIRE(value == emit.get_output(key));
  if (rank == 1) REQUIRE(value == null.get_input(key));
}

int main(int argc, char* argv[]) {
  MPI_Init(&argc, &argv);

  int result = Catch::Session().run(argc, argv);

  MPI_Finalize();

  return result;
}
