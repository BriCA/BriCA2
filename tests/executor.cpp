#include "catch.hpp"
#include "brica2/executors.hpp"

#include <chrono>
#include <thread>
#include <random>
#include <memory>

#include <iostream>

#define WORKLOAD 1000

namespace chrono = std::chrono;

using duration_type = chrono::milliseconds;

template <class D, class T> auto elapsed(T since) -> decltype(auto) {
  auto diff = chrono::steady_clock::now() - since;
  return chrono::duration_cast<D>(diff);
}

template <class T> class Workload {
 public:
  Workload(std::random_device& device)
      : engine(device()), x(std::make_shared<T>(T())) {}

  void operator()() {
    std::normal_distribution<T> dist;
    auto start = chrono::steady_clock::now();
    while (elapsed<duration_type>(start).count() < WORKLOAD) {
      (*x) += dist(engine);
    }
  }

  const T& get() const { return *x; }

 private:
  std::mt19937 engine;
  std::shared_ptr<T> x;
};

TEST_CASE("serial executor", "[serial]") {
  brica2::serial exec;

  std::random_device device;
  Workload<float> f0(device);
  Workload<float> f1(device);
  Workload<float> f2(device);

  auto start = chrono::steady_clock::now();
  exec.post(f0);
  exec.post(f1);
  exec.post(f2);
  exec.sync();
  auto time = elapsed<duration_type>(start);
  CHECK(time.count() >= WORKLOAD * 3);
  CHECK(f0.get() != float(0));
  CHECK(f1.get() != float(0));
  CHECK(f2.get() != float(0));
}

TEST_CASE("thread parallel executor", "[parallel]") {
  if (std::thread::hardware_concurrency() > 1) {
    brica2::parallel exec;

    std::random_device device;
    Workload<float> f0(device);
    Workload<float> f1(device);
    Workload<float> f2(device);

    auto start = chrono::steady_clock::now();
    exec.post(f0);
    exec.post(f1);
    exec.post(f2);
    exec.sync();
    auto time = elapsed<duration_type>(start);
    CHECK(time.count() < WORKLOAD * 3);
    CHECK(f0.get() != float(0));
    CHECK(f1.get() != float(0));
    CHECK(f2.get() != float(0));
  }
}
