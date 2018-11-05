#include "brica/brica.hpp"
#include "catch.hpp"

using namespace brica;

TEST_CASE("emit/pipe/null component chain", "[component]") {
  std::string key = "default";
  Buffer value({1, 2, 3});

  auto f_emit = [key, value](Dict& in, Dict& out) { out[key] = value; };
  auto f_pipe = [key](Dict& in, Dict& out) { out[key] = in[key]; };
  auto f_null = [](Dict& in, Dict& out) {};

  Component emit(f_emit);
  Component pipe(f_pipe);
  Component null(f_null);

  emit.make_out_port(key);
  pipe.make_in_port(key);
  pipe.make_out_port(key);
  null.make_in_port(key);

  connect(emit, key, pipe, key);
  connect(pipe, key, null, key);

  auto collect = [&]() {
    emit.collect();
    pipe.collect();
    null.collect();
  };

  auto execute = [&]() {
    emit.execute();
    pipe.execute();
    null.execute();
  };

  auto expose = [&]() {
    emit.expose();
    pipe.expose();
    null.expose();
  };

  REQUIRE(emit.get_output(key).empty());
  REQUIRE(pipe.get_input(key).empty());
  REQUIRE(pipe.get_output(key).empty());
  REQUIRE(null.get_input(key).empty());

  collect();
  execute();
  expose();

  REQUIRE(value == emit.get_output(key));
  REQUIRE(pipe.get_input(key).empty());
  REQUIRE(pipe.get_output(key).empty());
  REQUIRE(null.get_input(key).empty());

  collect();
  execute();
  expose();

  REQUIRE(value == emit.get_output(key));
  REQUIRE(value == pipe.get_input(key));
  REQUIRE(value == pipe.get_output(key));
  REQUIRE(null.get_input(key).empty());

  collect();
  execute();
  expose();

  REQUIRE(value == emit.get_output(key));
  REQUIRE(value == pipe.get_input(key));
  REQUIRE(value == pipe.get_output(key));
  REQUIRE(value == null.get_input(key));
}

TEST_CASE("emit/pipe/null fork component chain", "[component]") {
  std::string key = "default";
  Buffer value({1, 2, 3});

  auto f_emit = [key, value](Dict& in, Dict& out) { out[key] = value; };
  auto f_pipe = [key](Dict& in, Dict& out) { out[key] = in[key]; };
  auto f_null = [](Dict& in, Dict& out) {};

  ForkComponent emit(f_emit);
  ForkComponent pipe(f_pipe);
  ForkComponent null(f_null);

  emit.make_out_port(key);
  pipe.make_in_port(key);
  pipe.make_out_port(key);
  null.make_in_port(key);

  emit.run();
  pipe.run();
  null.run();

  connect(emit, key, pipe, key);
  connect(pipe, key, null, key);

  auto collect = [&]() {
    emit.collect();
    pipe.collect();
    null.collect();
  };

  auto execute = [&]() {
    emit.execute();
    pipe.execute();
    null.execute();
  };

  auto expose = [&]() {
    emit.expose();
    pipe.expose();
    null.expose();
  };

  REQUIRE(emit.get_output(key).empty());
  REQUIRE(pipe.get_input(key).empty());
  REQUIRE(pipe.get_output(key).empty());
  REQUIRE(null.get_input(key).empty());

  collect();
  execute();
  expose();

  REQUIRE(value == emit.get_output(key));
  REQUIRE(pipe.get_input(key).empty());
  REQUIRE(pipe.get_output(key).empty());
  REQUIRE(null.get_input(key).empty());

  collect();
  execute();
  expose();

  REQUIRE(value == emit.get_output(key));
  REQUIRE(value == pipe.get_input(key));
  REQUIRE(value == pipe.get_output(key));
  REQUIRE(null.get_input(key).empty());

  collect();
  execute();
  expose();

  REQUIRE(value == emit.get_output(key));
  REQUIRE(value == pipe.get_input(key));
  REQUIRE(value == pipe.get_output(key));
  REQUIRE(value == null.get_input(key));
}
