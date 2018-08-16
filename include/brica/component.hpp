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

#ifndef __BRICA_KERNEL_COMPONENT_HPP__
#define __BRICA_KERNEL_COMPONENT_HPP__

#include "brica/assocvec.hpp"
#include "brica/buffer.hpp"
#include "brica/functor.hpp"

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

template <class T, class D, class F>
class ComponentBase : public IComponent {
 public:
  class Port {
   public:
    void set(T& value) { buffer = value; }
    const T& get() const { return buffer; }

   private:
    T buffer;
  };

  using Ports = AssocVec<std::string, std::shared_ptr<Port>>;

  ComponentBase(F f) : functor(f) {}
  virtual ~ComponentBase() {}

  virtual void make_in_port(std::string name) {
    inputs.try_emplace(name, T());
    in_port.try_emplace(name, std::make_shared<Port>());
  }

  virtual void make_out_port(std::string name) {
    outputs.try_emplace(name, T());
    out_port.try_emplace(name, std::make_shared<Port>());
  }

  virtual void connect(ComponentBase& target, std::string from,
                       std::string to) {
    in_port.at(to) = target.out_port.at(from);
  }

  virtual void collect() {
    for (std::size_t i = 0; i < inputs.size(); ++i) {
      inputs.index(i) = in_port.index(i)->get();
    }
  }

  virtual void execute() { functor(inputs, outputs); }

  virtual void expose() {
    for (std::size_t i = 0; i < outputs.size(); ++i) {
      out_port.index(i)->set(outputs.index(i));
    }
  }

 protected:
  D inputs;
  D outputs;
  Ports in_port;
  Ports out_port;

 private:
  F functor;
};

class Component final : public ComponentBase<Buffer, Dict, Functor> {
 public:
  Component(Functor f) : ComponentBase<Buffer, Dict, Functor>(f) {}
  ~Component() {}

  const Buffer& get_in_port_value(std::string name) {
    return in_port.at(name)->get();
  }

  const Buffer& get_out_port_value(std::string name) {
    return out_port.at(name)->get();
  }

  Buffer& get_input(std::string name) { return inputs.at(name); }
  Buffer& get_output(std::string name) { return outputs.at(name); }
};

template <class C>
void connect(C& target, std::string from, C& origin, std::string to) {
  origin.connect(target, from, to);
}

}  // namespace brica

#endif  // __BRICA_KERNEL_COMPONENT_HPP__
