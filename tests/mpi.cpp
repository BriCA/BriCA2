#define CATCH_CONFIG_RUNNER
#include "brica/mpi.hpp"
#include "catch.hpp"

using namespace brica;

TEST_CASE("one to one communication", "[proxy]") {
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

  mpi::Proxy proxy(0, 1);

  REQUIRE(emit.get_output(key).empty());
  REQUIRE(proxy.get_input().empty());
  REQUIRE(proxy.get_buffer().empty());
  REQUIRE(proxy.get_output().empty());
  REQUIRE(null.get_input(key).empty());

  mpi::connect(emit, key, proxy);
  mpi::connect(proxy, null, key);

  REQUIRE(emit.get_out_port(key) == proxy.get_in_port());
  REQUIRE(null.get_in_port(key) == proxy.get_out_port());

  emit.execute();
  emit.expose();

  proxy.collect();
  proxy.execute();
  proxy.expose();

  null.collect();

  if (rank == 0) REQUIRE(value == emit.get_output(key));
  if (rank == 0) REQUIRE(value == proxy.get_input());
  if (rank == 1) REQUIRE(value == proxy.get_output());
  if (rank == 1) REQUIRE(value == null.get_input(key));
}

TEST_CASE("one to many communication", "[broadcast]") {
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  std::string key = "default";
  Buffer value({1, 2, 3});

  auto f_emit = [key, value](Dict& in, Dict& out) { out[key] = value; };
  auto f_null = [](Dict& in, Dict& out) {};

  mpi::Component emit(f_emit, 0);
  mpi::Component null0(f_null, 1);
  mpi::Component null1(f_null, 2);

  emit.make_out_port(key);
  null0.make_in_port(key);
  null1.make_in_port(key);

  mpi::Broadcast bcast(0);

  REQUIRE(emit.get_output(key).empty());
  REQUIRE(bcast.get_input().empty());
  REQUIRE(bcast.get_buffer().empty());
  REQUIRE(bcast.get_output().empty());
  REQUIRE(null0.get_input(key).empty());
  REQUIRE(null1.get_input(key).empty());

  mpi::connect(emit, key, bcast);
  mpi::connect(bcast, null0, key);
  mpi::connect(bcast, null1, key);

  REQUIRE(emit.get_out_port(key) == bcast.get_in_port());
  REQUIRE(null0.get_in_port(key) == bcast.get_out_port());
  REQUIRE(null1.get_in_port(key) == bcast.get_out_port());

  emit.execute();
  emit.expose();

  bcast.collect();
  bcast.execute();
  bcast.expose();

  null0.collect();
  null1.collect();

  if (rank == 0) REQUIRE(value == emit.get_output(key));
  if (rank == 0) REQUIRE(value == bcast.get_input());
  REQUIRE(value == bcast.get_output());
  if (rank == 1) REQUIRE(value == null0.get_input(key));
  if (rank == 2) REQUIRE(value == null1.get_input(key));
}

int main(int argc, char* argv[]) {
  MPI_Init(&argc, &argv);

  int result = Catch::Session().run(argc, argv);

  MPI_Finalize();

  return result;
}
