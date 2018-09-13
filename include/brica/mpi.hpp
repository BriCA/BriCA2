/******************************************************************************
 *
 * brica/mpi.hpp
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

class Component final : public ComponentBase<Buffer> {
 public:
  Component(Functor f, int want) : ComponentBase<Buffer>(f), want(want) {
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  }
  ~Component() {}

  void collect() {
    if (want == rank) {
      ComponentBase<Buffer>::collect();
    }
  }

  void execute() {
    if (want == rank) {
      ComponentBase<Buffer>::execute();
    }
  }

  void expose() {
    if (want == rank) {
      ComponentBase<Buffer>::expose();
    }
  }

  const Buffer& get_in_port_value(std::string name) {
    return in_ports.at(name)->get();
  }

  const Buffer& get_out_port_value(std::string name) {
    return out_ports.at(name)->get();
  }

  Buffer& get_input(std::string name) { return inputs.at(name); }
  Buffer& get_output(std::string name) { return outputs.at(name); }

 private:
  int want;
  int rank;
};

class Proxy final : public IComponent {
 public:
  Proxy(int src, int dest)
      : src(src),
        dest(dest),
        in_port(std::make_shared<Port<Buffer>>()),
        out_port(std::make_shared<Port<Buffer>>()) {
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  }
  ~Proxy() {}

  Buffer get_input() { return in_port->get(); }
  Buffer get_buffer() { return buffer; }
  Buffer get_output() { return out_port->get(); }

  std::shared_ptr<Port<Buffer>>& get_in_port() { return in_port; }
  std::shared_ptr<Port<Buffer>>& get_out_port() { return out_port; }

  void collect() {
    if (rank == src) {
      buffer = in_port->get();
    }
  }

  void execute() {
    if (rank == src) {
      int size = buffer.size();
      MPI_Send(&size, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
      MPI_Send(buffer.data(), size, MPI_CHAR, dest, 0, MPI_COMM_WORLD);
    }

    if (rank == dest) {
      int size;
      MPI_Recv(&size, 1, MPI_INT, src, 0, MPI_COMM_WORLD, &status);
      buffer.resize(size);
      MPI_Recv(buffer.data(), size, MPI_CHAR, src, 0, MPI_COMM_WORLD, &status);
    }
  }

  void expose() {
    if (rank == dest) {
      out_port->set(buffer);
    }
  }

  void connect_from(Component& source, std::string from) {
    in_port = source.get_out_port(from);
  }

  void connect_to(Component& target, std::string to) {
    target.get_in_port(to) = out_port;
  }

 private:
  int src;
  int dest;
  int rank;
  MPI_Status status;

  std::shared_ptr<Port<Buffer>> in_port;
  std::shared_ptr<Port<Buffer>> out_port;

  Buffer buffer;
};

class Broadcast final : public IComponent {
 public:
  Broadcast(int root)
      : root(root),
        in_port(std::make_shared<Port<Buffer>>()),
        out_port(std::make_shared<Port<Buffer>>()) {
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  }
  ~Broadcast() {}

  Buffer get_input() { return in_port->get(); }
  Buffer get_buffer() { return buffer; }
  Buffer get_output() { return out_port->get(); }

  std::shared_ptr<Port<Buffer>>& get_in_port() { return in_port; }
  std::shared_ptr<Port<Buffer>>& get_out_port() { return out_port; }

  void collect() {
    if (rank == root) {
      buffer = in_port->get();
    }
  }

  void execute() {
    int size = buffer.size();
    MPI_Bcast(&size, 1, MPI_INT, root, MPI_COMM_WORLD);

    if (rank != root) {
      buffer.resize(size);
    }

    MPI_Bcast(buffer.data(), buffer.size(), MPI_CHAR, root, MPI_COMM_WORLD);
  }

  void expose() { out_port->set(buffer); }

  void connect_from(Component& source, std::string from) {
    in_port = source.get_out_port(from);
  }

  void connect_to(Component& target, std::string to) {
    target.get_in_port(to) = out_port;
  }

 private:
  int root;
  int rank;

  std::shared_ptr<Port<Buffer>> in_port;
  std::shared_ptr<Port<Buffer>> out_port;

  Buffer buffer;
};

template <class P>
void connect(Component& source, std::string from, P& target) {
  target.connect_from(source, from);
}

template <class P>
void connect(P& source, Component& target, std::string to) {
  source.connect_to(target, to);
}

}  // namespace mpi
}  // namespace brica

#endif  // __KBRICA_MPI_COMPONENT_HPP__
