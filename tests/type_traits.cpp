#include "catch.hpp"
#include "brica2/type_traits.hpp"

TEST_CASE("Can find type", "[find_type]") {
  REQUIRE(brica2::find_type<short, short, int, long>() == 0);
  REQUIRE(brica2::find_type<int, short, int, long>() == 1);
  REQUIRE(brica2::find_type<long, short, int, long>() == 2);
}
