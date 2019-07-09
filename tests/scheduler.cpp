#include "brica2/brica2.hpp"
#include "brica2/executors.hpp"
#include "catch.hpp"

inline bool equal(const brica2::buffer& lhs, const brica2::buffer& rhs) {
  if (!compatible(lhs, rhs)) return false;
  auto lspan = lhs.as_span<float>();
  auto rspan = rhs.as_span<float>();
  return std::equal(lspan.begin(), lspan.end(), rspan.begin());
}

TEST_CASE(
    "emit/pipe/null component thread parallel scheduling", "[scheduler]") {
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

  brica2::parallel exec;
  brica2::virtual_time_scheduler s(exec);

  brica2::timing_t t{0, 1, 0};

  s.add(c1, t);
  s.add(c2, t);
  s.add(c3, t);

  CHECK(equal(c1.get_output(key), zeros));
  CHECK(equal(c2.get_input(key), zeros));
  CHECK(equal(c2.get_output(key), zeros));
  CHECK(equal(c3.get_input(key), zeros));

  CHECK(equal(c1.get_out_port(key).get(), zeros));
  CHECK(equal(c2.get_in_port(key).get(), zeros));
  CHECK(equal(c2.get_out_port(key).get(), zeros));
  CHECK(equal(c3.get_in_port(key).get(), zeros));

  s.step();

  CHECK(equal(c1.get_output(key), value));
  CHECK(equal(c2.get_input(key), zeros));
  CHECK(equal(c2.get_output(key), zeros));
  CHECK(equal(c3.get_input(key), zeros));

  CHECK(equal(c1.get_out_port(key).get(), zeros));
  CHECK(equal(c2.get_in_port(key).get(), zeros));
  CHECK(equal(c2.get_out_port(key).get(), zeros));
  CHECK(equal(c3.get_in_port(key).get(), zeros));

  s.step();

  CHECK(equal(c1.get_output(key), value));
  CHECK(equal(c2.get_input(key), value));
  CHECK(equal(c2.get_output(key), value));
  CHECK(equal(c3.get_input(key), zeros));

  CHECK(equal(c1.get_out_port(key).get(), value));
  CHECK(equal(c2.get_in_port(key).get(), value));
  CHECK(equal(c2.get_out_port(key).get(), zeros));
  CHECK(equal(c3.get_in_port(key).get(), zeros));

  s.step();

  CHECK(equal(c1.get_output(key), value));
  CHECK(equal(c2.get_input(key), value));
  CHECK(equal(c2.get_output(key), value));
  CHECK(equal(c3.get_input(key), value));

  CHECK(equal(c1.get_out_port(key).get(), value));
  CHECK(equal(c2.get_in_port(key).get(), value));
  CHECK(equal(c2.get_out_port(key).get(), value));
  CHECK(equal(c3.get_in_port(key).get(), value));

  for(std::size_t i = 0; i < 100; ++i) {
    s.step();
  }
}
