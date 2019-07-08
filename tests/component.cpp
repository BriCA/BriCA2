#include "catch.hpp"
#include "brica2/component.hpp"

#include <algorithm>
#include <vector>
#include <cstring>

inline bool equal(const brica2::buffer& lhs, const brica2::buffer& rhs) {
  if (!compatible(lhs, rhs)) return false;
  auto lspan = lhs.as_span<float>();
  auto rspan = rhs.as_span<float>();
  return std::equal(lspan.begin(), lspan.end(), rspan.begin());
}

TEST_CASE("constant/identity/discard component chain", "[component]") {
  std::string key = "default";
  std::vector<brica2::ssize_t> shape({3});
  auto zeros = brica2::fill<float>(shape, 0);
  auto value = brica2::with<float>({1, 2, 3}, shape);

  brica2::functor_type constant =
      [key, value](const auto& inputs, auto& outputs) { outputs[key] = value; };
  brica2::functor_type identity = [key](const auto& inputs, auto& outputs) {
    outputs[key] = inputs[key];
  };
  brica2::functor_type discard = [](const auto& inputs, auto& outputs) {};

  brica2::component c1(constant);
  brica2::component c2(identity);
  brica2::component c3(discard);

  c1.make_out_port<float>(key, shape);
  c2.make_in_port<float>(key, shape);
  c2.make_out_port<float>(key, shape);
  c3.make_in_port<float>(key, shape);

  brica2::connect({c1, key}, {c2, key});
  brica2::connect({c2, key}, {c3, key});

  auto collect = [&]() {
    c1.collect();
    c2.collect();
    c3.collect();
  };

  auto execute = [&]() {
    c1.execute();
    c2.execute();
    c3.execute();
  };

  auto expose = [&]() {
    c1.expose();
    c2.expose();
    c3.expose();
  };

  CHECK(equal(c1.get_output(key), zeros));
  CHECK(equal(c2.get_input(key), zeros));
  CHECK(equal(c2.get_output(key), zeros));
  CHECK(equal(c3.get_input(key), zeros));

  CHECK(equal(c1.get_out_port(key).get(), zeros));
  CHECK(equal(c2.get_in_port(key).get(), zeros));
  CHECK(equal(c2.get_out_port(key).get(), zeros));
  CHECK(equal(c3.get_in_port(key).get(), zeros));

  collect();
  execute();
  expose();

  CHECK(equal(c1.get_output(key), value));
  CHECK(equal(c2.get_input(key), zeros));
  CHECK(equal(c2.get_output(key), zeros));
  CHECK(equal(c3.get_input(key), zeros));

  CHECK(equal(c1.get_out_port(key).get(), value));
  CHECK(equal(c2.get_in_port(key).get(), value));
  CHECK(equal(c2.get_out_port(key).get(), zeros));
  CHECK(equal(c3.get_in_port(key).get(), zeros));

  collect();
  execute();
  expose();

  CHECK(equal(c1.get_output(key), value));
  CHECK(equal(c2.get_input(key), value));
  CHECK(equal(c2.get_output(key), value));
  CHECK(equal(c3.get_input(key), zeros));

  CHECK(equal(c1.get_out_port(key).get(), value));
  CHECK(equal(c2.get_in_port(key).get(), value));
  CHECK(equal(c2.get_out_port(key).get(), value));
  CHECK(equal(c3.get_in_port(key).get(), value));

  collect();
  execute();
  expose();

  CHECK(equal(c1.get_output(key), value));
  CHECK(equal(c2.get_input(key), value));
  CHECK(equal(c2.get_output(key), value));
  CHECK(equal(c3.get_input(key), value));

  CHECK(equal(c1.get_out_port(key).get(), value));
  CHECK(equal(c2.get_in_port(key).get(), value));
  CHECK(equal(c2.get_out_port(key).get(), value));
  CHECK(equal(c3.get_in_port(key).get(), value));
}
