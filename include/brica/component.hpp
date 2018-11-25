/******************************************************************************
 *
 * brica/component.hpp
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *****************************************************************************/

#ifndef __BRICA_COMPONENT_HPP__
#define __BRICA_COMPONENT_HPP__

#include "brica/assocvec.hpp"
#include "brica/buffer.hpp"
#include "brica/functor.hpp"
#include "brica/port.hpp"

#include <string>
#include <utility>

namespace brica {

class IComponent {
 public:
  virtual ~IComponent() {}
  virtual void collect() = 0;
  virtual void execute() = 0;
  virtual void expose() = 0;
};

template <class T>
class ComponentBase : public IComponent {
  using D = AssocVec<std::string, T>;
  using F = std::function<void(D&, D&)>;

 public:
  ComponentBase(F f) : functor(f) {}
  virtual ~ComponentBase() {}

  virtual void make_in_port(std::string name) {
    inputs.try_emplace(name, T());
    in_ports.try_emplace(name, std::make_shared<Port<T>>());
  }

  virtual std::shared_ptr<Port<T>>& get_in_port(std::string name) {
    return in_ports.at(name);
  }

  virtual void make_out_port(std::string name) {
    outputs.try_emplace(name, T());
    out_ports.try_emplace(name, std::make_shared<Port<T>>());
  }

  virtual std::shared_ptr<Port<T>>& get_out_port(std::string name) {
    return out_ports.at(name);
  }

  virtual void connect(ComponentBase& target, std::string out, std::string in) {
    in_ports.at(in) = target.out_ports.at(out);
  }

  virtual void collect() {
    for (std::size_t i = 0; i < inputs.size(); ++i) {
      inputs.index(i) = in_ports.index(i)->get();
    }
  }

  virtual void execute() { functor(inputs, outputs); }

  virtual void expose() {
    for (std::size_t i = 0; i < outputs.size(); ++i) {
      out_ports.index(i)->set(outputs.index(i));
    }
  }

 protected:
  D inputs;
  D outputs;
  Ports<T> in_ports;
  Ports<T> out_ports;

 private:
  F functor;
};

class Component final : public ComponentBase<Buffer> {
 public:
  Component(Functor f) : ComponentBase<Buffer>(f) {}
  ~Component() {}

  const Buffer& get_in_port_value(std::string name) {
    return in_ports.at(name)->get();
  }

  const Buffer& get_out_port_value(std::string name) {
    return out_ports.at(name)->get();
  }

  Buffer& get_input(std::string name) { return inputs.at(name); }
  Buffer& get_output(std::string name) { return outputs.at(name); }
};

template <class C>
void connect(C& target, std::string out, C& origin, std::string in) {
  origin.connect(target, out, in);
}

}  // namespace brica

#endif  // __BRICA_COMPONENT_HPP__
