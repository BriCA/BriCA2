#include "brica/brica.hpp"
#include "gtest/gtest.h"

using namespace brica;

TEST(component, emit_pipe_null) {
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

  ASSERT_TRUE(emit.get_output(key).empty());
  ASSERT_TRUE(pipe.get_input(key).empty());
  ASSERT_TRUE(pipe.get_output(key).empty());
  ASSERT_TRUE(null.get_input(key).empty());

  collect();
  execute();
  expose();

  ASSERT_EQ(value, emit.get_output(key));
  ASSERT_TRUE(pipe.get_input(key).empty());
  ASSERT_TRUE(pipe.get_output(key).empty());
  ASSERT_TRUE(null.get_input(key).empty());

  collect();
  execute();
  expose();

  ASSERT_EQ(value, emit.get_output(key));
  ASSERT_EQ(value, pipe.get_input(key));
  ASSERT_EQ(value, pipe.get_output(key));
  ASSERT_TRUE(null.get_input(key).empty());

  collect();
  execute();
  expose();

  ASSERT_EQ(value, emit.get_output(key));
  ASSERT_EQ(value, pipe.get_input(key));
  ASSERT_EQ(value, pipe.get_output(key));
  ASSERT_EQ(value, null.get_input(key));
}
