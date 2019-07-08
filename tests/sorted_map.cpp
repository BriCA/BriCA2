#include "catch.hpp"
#include "brica2/sorted_map.hpp"

#include <string>

TEST_CASE("constructible", "[sorted_map]") {
  SECTION("default construction") {
    REQUIRE_NOTHROW(
        []() { brica2::sorted_map<std::string, int> map; });
  }
}

TEST_CASE("preserves key ordering", "[sorted_map]") {
  brica2::sorted_map<std::string, int> map;

  SECTION("insert in alphabetical order") {
    map.insert({"alice", 0});
    map.insert({"bob", 1});
    map.insert({"charlie", 2});
    map.insert({"diana", 3});

    REQUIRE(map.index(0) == 0);
    REQUIRE(map.index(1) == 1);
    REQUIRE(map.index(2) == 2);
    REQUIRE(map.index(3) == 3);
  }

  SECTION("insert in reverse alphabetical order") {
    map.insert({"diana", 3});
    map.insert({"charlie", 2});
    map.insert({"bob", 1});
    map.insert({"alice", 0});

    REQUIRE(map.index(0) == 0);
    REQUIRE(map.index(1) == 1);
    REQUIRE(map.index(2) == 2);
    REQUIRE(map.index(3) == 3);
  }
}
