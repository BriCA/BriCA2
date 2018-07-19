#include "brica/mpi.hpp"
#include "mpi.h"

#include <chrono>
#include <random>
#include <vector>

#include <iostream>

using namespace std::chrono;

int main(int argc, char* argv[]) {
  MPI_Init(&argc, &argv);

  int size, rank;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  brica::Buffer payload(1000);

  std::random_device seed_gen;
  std::mt19937 engine(seed_gen());
  std::normal_distribution<> distribution;

  brica::Functor f = [&](brica::Dict& inputs, brica::Dict& outputs) {
    auto start = steady_clock::now();

    double value = 0.0;
    for (auto end = start;
         duration_cast<microseconds>(end - start).count() < 1000;
         end = steady_clock::now()) {
      value += distribution(engine);
    }

    outputs["default"] = payload;
  };

  std::vector<brica::mpi::Component*> components(size);

  brica::mpi::VirtualTimeScheduler scheduler;
  brica::Timing timing{0, 1, 0};

  for (int i = 0; i < size; ++i) {
    components[i] = new brica::mpi::Component(f, i);
    components[i]->make_in_port("default");
    components[i]->make_out_port("default");
    if (i > 0) {
      brica::connect(*components[i - 1], "default", *components[i], "default");
    }

    scheduler.add_component(components[i], timing);
  }

  auto start = steady_clock::now();

  std::size_t i = 0;

  for (auto end = start;
       duration_cast<milliseconds>(end - start).count() < 5000;
       end = steady_clock::now()) {
    scheduler.step();
    ++i;
  }

  MPI_Barrier(MPI_COMM_WORLD);

  if (rank == 0) {
    std::cout
        << duration_cast<microseconds>(steady_clock::now() - start).count() / i
        << std::endl;
  }

  for (int i = 0; i < size; ++i) {
    delete components[i];
  }

  MPI_Finalize();
}
