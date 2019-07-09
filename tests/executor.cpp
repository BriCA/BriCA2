#include "catch.hpp"
#include "brica2/executors.hpp"

#include <chrono>
#include <thread>

namespace chrono = std::chrono;

TEST_CASE("serial executor", "[serial]") {
  brica2::serial exec;

  auto sleep = [] { std::this_thread::sleep_for(chrono::milliseconds(1)); };
  auto start = chrono::steady_clock::now();
  exec.post({sleep, sleep, sleep});
  auto end = chrono::steady_clock::now();
  auto diff = chrono::duration_cast<chrono::milliseconds>(end - start);
  REQUIRE(diff.count() == 3);
}

TEST_CASE("thread parallel executor", "[parallel][!mayfail]") {
  brica2::parallel exec;

  auto sleep = [] { std::this_thread::sleep_for(chrono::milliseconds(1)); };
  auto start = chrono::steady_clock::now();
  exec.post({sleep, sleep, sleep});
  auto end = chrono::steady_clock::now();
  auto diff = chrono::duration_cast<chrono::milliseconds>(end - start);
  REQUIRE(diff.count() < 3);
}
