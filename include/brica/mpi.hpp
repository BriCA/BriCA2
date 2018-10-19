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

class Component final : public IComponent {
 public:
  Component(Functor f, int want) : base(f), want(want) {
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  }
  ~Component() {}

  void collect() {
    if (want == rank) {
      base.collect();
    }
  }

  void execute() {
    if (want == rank) {
      base.execute();
    }
  }

  void expose() {
    if (want == rank) {
      base.expose();
    }
  }

  void make_in_port(std::string name) { base.make_in_port(name); }

  std::shared_ptr<Port<Buffer>>& get_in_port(std::string name) {
    return base.get_in_port(name);
  }

  void make_out_port(std::string name) { base.make_out_port(name); }

  std::shared_ptr<Port<Buffer>>& get_out_port(std::string name) {
    return base.get_out_port(name);
  }

  const Buffer& get_in_port_value(std::string name) {
    return base.get_in_port_value(name);
  }

  const Buffer& get_out_port_value(std::string name) {
    return base.get_out_port_value(name);
  }

  Buffer& get_input(std::string name) { return base.get_input(name); }
  Buffer& get_output(std::string name) { return base.get_output(name); }

  void connect(Component& target, std::string out, std::string in) {
    base.connect(target.base, out, in);
  }

 private:
  brica::Component base;

  int want;
  int rank;
};

class Proxy final : public IComponent {
 public:
  Proxy(int src, int dest, int tag = 0)
      : src(src),
        dest(dest),
        tag(tag),
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
      MPI_Send(buffer.data(), buffer.size(), MPI_CHAR, dest, tag,
               MPI_COMM_WORLD);
    }

    if (rank == dest) {
      MPI_Probe(src, tag, MPI_COMM_WORLD, &status);
      int size;
      MPI_Get_count(&status, MPI_CHAR, &size);
      buffer.resize(size);
      MPI_Recv(buffer.data(), buffer.size(), MPI_CHAR, src, tag, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
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
  int tag;
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
    int size;

    if (rank == root) {
      size = buffer.size();
    }

    MPI_Bcast(&size, 1, MPI_INT, root, MPI_COMM_WORLD);

    if (size == 0) {
      return;
    }

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
