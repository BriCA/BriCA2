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
  virtual void make_in_port(std::string) = 0;
  virtual void make_out_port(std::string) = 0;
  virtual Buffer& get_input(std::string) = 0;
  virtual Buffer& get_output(std::string) = 0;
  virtual void collect() = 0;
  virtual void execute() = 0;
  virtual void expose() = 0;
};

class Component : public IComponent {
 public:
  class Port {
   public:
    void send(Buffer& value) { set(value); }
    const Buffer& recv() const { return get(); }

    void set(Buffer& value) { buffer = value; }
    const Buffer& get() const { return buffer; }

   private:
    Buffer buffer;
  };

  using Ports = AssocVec<std::string, std::shared_ptr<Port>>;

  Component(Functor f) : functor(f) {}

  void make_in_port(std::string name) {
    inputs.try_emplace(name, Buffer());
    in_port.try_emplace(name, std::make_shared<Port>());
  }

  const Buffer& get_in_port_buffer(std::string name) {
    return in_port.at(name)->get();
  }

  void make_out_port(std::string name) {
    outputs.try_emplace(name, Buffer());
    out_port.try_emplace(name, std::make_shared<Port>());
  }

  const Buffer& get_out_port_buffer(std::string name) {
    return out_port.at(name)->get();
  }

  Buffer& get_input(std::string name) { return inputs.at(name); }
  Buffer& get_output(std::string name) { return outputs.at(name); }

  void connect(Component& target, std::string from, std::string to) {
    in_port[to] = target.out_port[from];
  }

  void collect() {
    for (std::size_t i = 0; i < inputs.size(); ++i) {
      inputs.index(i) = in_port.index(i)->recv();
    }
  }

  void execute() { functor(inputs, outputs); }

  void expose() {
    for (std::size_t i = 0; i < outputs.size(); ++i) {
      out_port.index(i)->send(outputs.index(i));
    }
  }

 private:
  Functor functor;
  Dict inputs;
  Dict outputs;
  Ports in_port;
  Ports out_port;
};

template <class C>
void connect(C& target, std::string from, C& origin, std::string to) {
  origin.connect(target, from, to);
}

}  // namespace brica

#endif  // __BRICA_KERNEL_COMPONENT_HPP__
