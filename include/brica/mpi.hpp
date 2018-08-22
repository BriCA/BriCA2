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

class Proxy;

class Component final : public IComponent {
  friend Proxy;

 public:
  Component(Functor f, int want = 0) : base(f), want(want) {
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  }

  ~Component() {}

  void make_in_port(std::string name) { base.make_in_port(name); }

  std::shared_ptr<Port<Buffer>> get_in_port(std::string name) {
    return base.get_in_port(name);
  }

  const Buffer& get_in_port_value(std::string name) {
    return base.get_in_port_value(name);
  }

  void make_out_port(std::string name) { base.make_out_port(name); }

  std::shared_ptr<Port<Buffer>> get_out_port(std::string name) {
    return base.get_out_port(name);
  }

  const Buffer& get_out_port_value(std::string name) {
    return base.get_out_port_value(name);
  }

  Buffer& get_input(std::string name) { return base.get_input(name); }
  Buffer& get_output(std::string name) { return base.get_output(name); }

  void connect(Component& target, std::string from, std::string to) {
    base.connect(target.base, from, to);
  }

  void collect() {
    if (rank == want) {
      base.collect();
    }
  }

  void execute() {
    if (rank == want) {
      base.execute();
    }
  }

  void expose() {
    if (rank == want) {
      base.expose();
    }
  }

 private:
  brica::Component base;
  int want;
  int rank;
};

class Proxy final : public IComponent {
 public:
  Proxy(Component& a, std::string out, Component& b, std::string in)
      : origin(a.get_out_port(out)),
        target(b.get_in_port(in)),
        src(a.want),
        dest(b.want) {
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  }
  ~Proxy() {}

  void collect() {
    if (rank == src && rank == dest) {
      target->set(origin->get());
    } else {
      if (rank == src) {
        Buffer buffer = origin->get();
        int count = buffer.size();
        MPI_Send(&count, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
        MPI_Send(buffer.data(), count, MPI_CHAR, dest, 0, MPI_COMM_WORLD);
      }

      if (rank == dest) {
        int count;
        MPI_Recv(&count, 1, MPI_INT, src, 0, MPI_COMM_WORLD, &status);
        Buffer buffer(count);
        MPI_Recv(buffer.data(), count, MPI_CHAR, src, 0, MPI_COMM_WORLD,
                 &status);
        target->set(buffer);
      }
    }
  }

  void execute() {}

  void expose() {}

 private:
  std::shared_ptr<Port<Buffer>> origin;
  std::shared_ptr<Port<Buffer>> target;

  int src;
  int dest;
  int rank;

  MPI_Status status;
};

class VirtualTimeSyncScheduler {
 public:
  VirtualTimeSyncScheduler(MPI_Comm comm = MPI_COMM_WORLD) : comm(comm) {}

  void add_component(IComponent* component) { components.push_back(component); }

  void step() {
    for (std::size_t i = 0; i < components.size(); ++i) {
      components[i]->collect();
      components[i]->execute();
    }

    for (std::size_t i = 0; i < components.size(); ++i) {
      components[i]->expose();
    }

    MPI_Barrier(comm);
  }

 private:
  MPI_Comm comm;

  std::vector<IComponent*> components;
};

class VirtualTimeScheduler {
  struct Event {
    Time time;
    IComponent* component;
    Timing timing;
    bool sleep;

    bool operator<(const Event& rhs) const { return time > rhs.time; }
  };

 public:
  void add_component(IComponent* component, Timing timing) {
    event_queue.push({timing.offset, component, timing, false});
  }

  void step() {
    Time time = event_queue.top().time;

    std::queue<IComponent*> awake;
    std::queue<IComponent*> asleep;

    while (event_queue.top().time == time) {
      Event event = event_queue.top();
      event_queue.pop();

      Event next = event;

      if (event.sleep) {
        asleep.push(event.component);
        next.time += next.timing.sleep;
        next.sleep = false;
      } else {
        awake.push(event.component);
        next.time += next.timing.interval;
        next.sleep = true;
      }

      event_queue.push(next);
    }

    while (!asleep.empty()) {
      asleep.front()->expose();
      asleep.pop();
    }

    MPI_Barrier(MPI_COMM_WORLD);

    while (!awake.empty()) {
      awake.front()->collect();
      awake.front()->execute();
      awake.pop();
    }
  }

 private:
  std::priority_queue<Event> event_queue;
};

}  // namespace mpi
}  // namespace brica

#endif  // __KBRICA_MPI_COMPONENT_HPP__
