#include "brica/mpi.hpp"
#include "gtest/gtest.h"

using namespace brica;

class MPITest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    char** argv;
    int argc = 0;
    int mpiError = MPI_Init(&argc, &argv);
    ASSERT_FALSE(mpiError);
  }

  virtual void TearDown() {
    int mpiError = MPI_Finalize();
    ASSERT_FALSE(mpiError);
  }

  virtual ~MPITest() {};
};

TEST_F(MPITest, emit_pipe_null) {
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  std::string key = "default";
  Buffer value({1, 2, 3});

  auto f_emit = [key, value](Dict& in, Dict& out) { out[key] = value; };
  auto f_pipe = [key](Dict& in, Dict& out) { out[key] = in[key]; };
  auto f_null = [](Dict& in, Dict& out) {};

  mpi::Component emit(f_emit, 0);
  mpi::Component pipe(f_pipe, 1);
  mpi::Component null(f_null, 2);

  emit.make_out_port(key);
  pipe.make_in_port(key);
  pipe.make_out_port(key);
  null.make_in_port(key);

  connect(emit, key, pipe, key);
  connect(pipe, key, null, key);

  VirtualTimeScheduler s;
  Timing t({0, 1, 0});

  s.add_component(&emit, t);
  s.add_component(&pipe, t);
  s.add_component(&null, t);

  if(rank == 0) {
    EXPECT_TRUE(emit.get_output(key).empty());
    EXPECT_TRUE(emit.get_out_port_buffer(key).empty());
  }

  if(rank == 1) {
    EXPECT_TRUE(pipe.get_input(key).empty());
    EXPECT_TRUE(pipe.get_in_port_buffer(key).empty());

    EXPECT_TRUE(pipe.get_output(key).empty());
    EXPECT_TRUE(pipe.get_out_port_buffer(key).empty());
  }

  if(rank == 2) {
    EXPECT_TRUE(null.get_input(key).empty());
    EXPECT_TRUE(null.get_in_port_buffer(key).empty());
  }

  s.step();

  if(rank == 0) {
    EXPECT_EQ(value, emit.get_output(key));
    EXPECT_TRUE(emit.get_out_port_buffer(key).empty());
  }

  if(rank == 1) {
    EXPECT_TRUE(pipe.get_input(key).empty());
    EXPECT_TRUE(pipe.get_in_port_buffer(key).empty());

    EXPECT_TRUE(pipe.get_output(key).empty());
    EXPECT_TRUE(pipe.get_out_port_buffer(key).empty());
  }

  if(rank == 2) {
    EXPECT_TRUE(null.get_input(key).empty());
    EXPECT_TRUE(null.get_in_port_buffer(key).empty());
  }

  s.step();

  if(rank == 0) {
    EXPECT_EQ(value, emit.get_output(key));
    EXPECT_EQ(value, emit.get_out_port_buffer(key));
  }

  if(rank == 1) {
    EXPECT_EQ(value, pipe.get_input(key));
    EXPECT_EQ(value, pipe.get_in_port_buffer(key));

    EXPECT_EQ(value, pipe.get_output(key));
    EXPECT_TRUE(pipe.get_out_port_buffer(key).empty());
  }

  if(rank == 2) {
    EXPECT_TRUE(null.get_input(key).empty());
    EXPECT_TRUE(null.get_in_port_buffer(key).empty());
  }

  s.step();


  if(rank == 0) {
    EXPECT_EQ(value, emit.get_output(key));
    EXPECT_EQ(value, emit.get_out_port_buffer(key));
  }

  if(rank == 1) {
    EXPECT_EQ(value, pipe.get_input(key));
    EXPECT_EQ(value, pipe.get_in_port_buffer(key));

    EXPECT_EQ(value, pipe.get_output(key));
    EXPECT_EQ(value, pipe.get_out_port_buffer(key));
  }

  if(rank == 2) {
    EXPECT_EQ(value, null.get_input(key));
    EXPECT_EQ(value, null.get_in_port_buffer(key));
  }
}

