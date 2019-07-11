#ifndef __BRICA2_COMPONENT_HPP__
#define __BRICA2_COMPONENT_HPP__

#include "brica2/buffer.hpp"
#include "brica2/format.hpp"
#include "brica2/port.hpp"
#include "brica2/sorted_map.hpp"

#include <functional>

namespace brica2 {

struct component_type {
  virtual void collect() = 0;
  virtual void execute() = 0;
  virtual void expose() = 0;
};

class dictionary : public sorted_map<std::string, buffer> {};
using functor_type = std::function<void(const dictionary&, dictionary&)>;

class basic_component : public component_type {
 public:
  basic_component() = delete;

  explicit basic_component(const functor_type& f) : functor(f) {}
  explicit basic_component(functor_type&& f) : functor(f) {}

  basic_component(const basic_component&) = default;
  basic_component(basic_component&&) = default;
  basic_component& operator=(const basic_component&) = default;
  basic_component& operator=(basic_component&&) = default;

  template <class T, class S = std::initializer_list<ssize_t>>
  void make_in_port(const std::string& key, S&& s) {
    in_ports.try_emplace(key, std::forward<S>(s), T());
    inputs.try_emplace(key, fill(std::forward<S>(s), T()));
  }

  template <class T, class S = std::initializer_list<ssize_t>>
  void make_out_port(const std::string& key, S&& s) {
    out_ports.try_emplace(key, std::forward<S>(s), T());
    outputs.try_emplace(key, fill(std::forward<S>(s), T()));
  }

  port& get_in_port(const std::string& key) { return in_ports.at(key); }
  port& get_out_port(const std::string& key) { return out_ports.at(key); }

  buffer& get_input(const std::string& key) { return inputs.at(key); }
  buffer& get_output(const std::string& key) { return outputs.at(key); }

  virtual void collect() override {
    for (std::size_t i = 0; i < in_ports.size(); ++i) {
      if (!compatible(inputs.index(i), in_ports.index(i).get())) {
        throw incompatible_exception();
      }
      inputs.index(i) = in_ports.index(i).get();
    }
  }

  virtual void execute() override {
    for (std::size_t i = 0; i < outputs.size(); ++i) {
      outputs.index(i) = zeros_like(outputs.index(i));
    }
    functor(inputs, outputs);
  }

  virtual void expose() override {
    for (std::size_t i = 0; i < out_ports.size(); ++i) {
      if (!compatible(outputs.index(i), out_ports.index(i).get())) {
        throw incompatible_exception();
      }
      out_ports.index(i).get() = outputs.index(i);
    }
  }

 private:
  functor_type functor;

  sorted_map<std::string, port> in_ports;
  sorted_map<std::string, port> out_ports;

  dictionary inputs;
  dictionary outputs;
};

using component = basic_component;

struct port_spec {
  component& c;
  std::string k;
};

inline void connect(port_spec&& source, port_spec&& target) {
  auto& source_port = source.c.get_out_port(source.k);
  auto& target_port = target.c.get_in_port(target.k);
  if (!compatible(source_port.get(), target_port.get())) {
    throw incompatible_exception();
  }
  target_port = source_port;
}

}  // namespace brica2

#endif  // __BRICA2_COMPONENT_HPP__
