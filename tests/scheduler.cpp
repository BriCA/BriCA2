#include "brica/brica.hpp"
#include "catch.hpp"

using namespace brica;

TEST_CASE("emit/pipe/null component scheduling", "[scheduler]") {
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

  VirtualTimeScheduler s;
  Timing t({0, 1, 0});

  s.add_component(&emit, t);
  s.add_component(&pipe, t);
  s.add_component(&null, t);

  CHECK(emit.get_output(key).empty());
  CHECK(emit.get_out_port_buffer(key).empty());

  CHECK(pipe.get_input(key).empty());
  CHECK(pipe.get_in_port_buffer(key).empty());

  CHECK(pipe.get_output(key).empty());
  CHECK(pipe.get_out_port_buffer(key).empty());

  CHECK(null.get_input(key).empty());
  CHECK(null.get_in_port_buffer(key).empty());

  s.step();

  CHECK(value == emit.get_output(key));
  CHECK(emit.get_out_port_buffer(key).empty());

  CHECK(pipe.get_input(key).empty());
  CHECK(pipe.get_in_port_buffer(key).empty());

  CHECK(pipe.get_output(key).empty());
  CHECK(pipe.get_out_port_buffer(key).empty());

  CHECK(null.get_input(key).empty());
  CHECK(null.get_in_port_buffer(key).empty());

  s.step();

  CHECK(value == emit.get_output(key));
  CHECK(value == emit.get_out_port_buffer(key));

  CHECK(value == pipe.get_input(key));
  CHECK(value == pipe.get_in_port_buffer(key));

  CHECK(value == pipe.get_output(key));
  CHECK(pipe.get_out_port_buffer(key).empty());

  CHECK(null.get_input(key).empty());
  CHECK(null.get_in_port_buffer(key).empty());

  s.step();

  CHECK(value == emit.get_output(key));
  CHECK(value == emit.get_out_port_buffer(key));

  CHECK(value == pipe.get_input(key));
  CHECK(value == pipe.get_in_port_buffer(key));

  CHECK(value == pipe.get_output(key));
  CHECK(value == pipe.get_out_port_buffer(key));

  CHECK(value == null.get_input(key));
  CHECK(value == null.get_in_port_buffer(key));
}
