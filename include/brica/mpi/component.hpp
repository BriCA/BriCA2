/******************************************************************************
 *
 * brica/mpi/component.hpp
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

#ifndef __KBRICA_MPI_COMPONENT_HPP__
#define __KBRICA_MPI_COMPONENT_HPP__

#include "brica/brica.hpp"

#include "mpi.h"

namespace brica {
namespace mpi {

class Component : IComponent {
  class Port {
   public:
    Port(int rank, int tag = 0) : target(rank), tag(tag) {}

    void sync(int wanted, int actual) {
      int size;
      if (wanted == actual && target != wanted) {
        size = buffer.size();
        MPI_Send(&size, 1, MPI_INT, target, tag, MPI_COMM_WORLD);
        if (size > 1) {
          char* ptr = buffer.data();
          MPI_Send(ptr, size, MPI_CHAR, target, tag, MPI_COMM_WORLD);
        }
      }

      if (wanted != actual && target == actual) {
        MPI_Recv(&size, 1, MPI_INT, wanted, tag, MPI_COMM_WORLD, &status);
        buffer.resize(size);
        if (size > 1) {
          char* ptr = buffer.data();
          MPI_Recv(ptr, size, MPI_CHAR, wanted, tag, MPI_COMM_WORLD, &status);
        }
      }
    }

    void set(Buffer& value) { buffer = value; }
    const Buffer& get() const { return buffer; }

   private:
    Buffer buffer;
    int target;
    int tag;

    MPI_Status status;
  };

  using Ports = AssocVec<std::string, std::shared_ptr<Port>>;

 public:
  Component(Functor f, int rank) : f(f), wanted(rank) {
    MPI_Comm_rank(MPI_COMM_WORLD, &actual);
  }

  void make_in_port(std::string name) {
    inputs.try_emplace(name, Buffer());
    in_port.try_emplace(name, std::make_shared<Port>(actual));
  }

  void make_out_port(std::string name) {
    outputs.try_emplace(name, Buffer());
    out_port.try_emplace(name, std::make_shared<Port>(actual));
  }

  Buffer& get_input(std::string name) { return inputs.at(name); }
  Buffer& get_output(std::string name) { return outputs.at(name); }

  void connect(Component& target, std::string from, std::string to) {
    in_port[to] = target.out_port[from];
  }

  void collect() {
    if (wanted == actual) {
      for (std::size_t i = 0; i < inputs.size(); ++i) {
        inputs.index(i) = in_port.index(i)->get();
      }
    }
  }

  void execute() {
    if (wanted == actual) {
      f(inputs, outputs);
    }
  }

  void expose() {
    for (std::size_t i = 0; i < outputs.size(); ++i) {
      auto port = out_port.index(i);
      port->set(outputs.index(i));
      port->sync(wanted, actual);
    }
  }

 private:
  Functor f;
  int wanted;
  int actual;

  Dict inputs;
  Dict outputs;
  Ports in_port;
  Ports out_port;
};

}  // namespace mpi
}  // namespace brica

#endif  // __KBRICA_MPI_COMPONENT_HPP__
