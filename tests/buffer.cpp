#include "brica2/buffer.hpp"
#include "catch.hpp"

#include <algorithm>
#include <vector>

TEST_CASE("generators", "[buffer]") {
  SECTION("empty buffer") {
    auto b = brica2::empty<float>({2, 3, 4});
    auto& i = b.request();

    brica2::ssize_t size = sizeof(float);

    std::vector<brica2::ssize_t> shape = {2, 3, 4};
    std::vector<brica2::ssize_t> strides = {3 * 4 * size, 4 * size, size};

    REQUIRE(i.itemsize == size);
    REQUIRE(i.format == "f");
    REQUIRE(i.ndim == 3);
    REQUIRE(std::equal(i.shape.begin(), i.shape.end(), shape.begin()));
    REQUIRE(std::equal(i.strides.begin(), i.strides.end(), strides.begin()));
    REQUIRE(i.ptr != nullptr);
  }

  SECTION("filled buffer") {
    auto b = brica2::fill<float>({2, 3, 4}, 42.0);
    auto& i = b.request();

    brica2::ssize_t size = sizeof(float);

    std::vector<brica2::ssize_t> shape = {2, 3, 4};
    std::vector<brica2::ssize_t> strides = {3 * 4 * size, 4 * size, size};
    std::vector<float> v(b.size(), 42.0);

    REQUIRE(i.itemsize == size);
    REQUIRE(i.format == "f");
    REQUIRE(i.ndim == 3);
    REQUIRE(std::equal(i.shape.begin(), i.shape.end(), shape.begin()));
    REQUIRE(std::equal(i.strides.begin(), i.strides.end(), strides.begin()));
    REQUIRE(i.ptr != nullptr);
    REQUIRE(std::equal(v.begin(), v.end(), reinterpret_cast<float*>(i.ptr)));
  }

  SECTION("compatibility") {
    auto b0 = brica2::empty<float>({2, 3, 4});
    auto b1 = brica2::empty<float>({2, 3, 4});
    auto b2 = brica2::empty<float>({2, 2});

    REQUIRE(b0.data() != b1.data());
    REQUIRE(b0.data() != b2.data());
    REQUIRE(b1.data() != b2.data());

    REQUIRE(brica2::compatible(b0, b1));
    REQUIRE(!brica2::compatible(b0, b2));
    REQUIRE(!brica2::compatible(b1, b2));
  }
}
