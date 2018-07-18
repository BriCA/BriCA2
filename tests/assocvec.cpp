#include "brica/assocvec.hpp"
#include "catch.hpp"

TEST_CASE("AssocVec key/index access", "[AssocVec]") {
  brica::AssocVec<int, int> map;

  int i;
  for (i = 0; i < 10; ++i) {
    map[i] = i;
  }

  for (i = 0; i < 10; ++i) {
    REQUIRE(i == map[i]);
    REQUIRE(i == map.index(i));
  }
}

TEST_CASE("AssocVec reverse sort key/index access", "[AssocVec]") {
  brica::AssocVec<int, int, std::greater<int>> map;

  int i;
  for (i = 0; i < 10; ++i) {
    map[i] = i;
  }

  for (i = 0; i < 10; ++i) {
    REQUIRE(i == map[i]);
    REQUIRE(9 - i == map.index(i));
  }
}
