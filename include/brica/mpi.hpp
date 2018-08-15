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

class Component : public IComponent {
  struct Port {
    Port(MPI_Comm comm, int tag = 0)
        : comm(comm), tag(tag), request(MPI_REQUEST_NULL) {}

    ~Port() { MPI_Wait(&request, &status); }

    void sync(int src, int actual) {
      for (int i = 0; i < dests.size(); ++i) {
        int dest = dests[i];
        int size, flag = 1;

        if (src == actual && dest != src) {
          MPI_Test(&request, &flag, &status);
          if (flag) {
            size = buffer.size();
            MPI_Send(&size, 1, MPI_INT, dest, tag, comm);
            if (size > 1) {
              char* ptr = buffer.data();
              MPI_Isend(ptr, size, MPI_CHAR, dest, tag, comm, &request);
            }
          }
        }

        if (src != actual && dest == actual) {
          MPI_Test(&request, &flag, &status);
          if (flag) {
            MPI_Recv(&size, 1, MPI_INT, src, tag, comm, &status);
            buffer.resize(size);
            if (size > 1) {
              char* ptr = buffer.data();
              MPI_Irecv(ptr, size, MPI_CHAR, src, tag, comm, &request);
            }
          }
        }
      }
    }

    void set(Buffer& value) { buffer = value; }
    const Buffer& get() const { return buffer; }

    void add_dest(int rank) {
      if (std::find(dests.begin(), dests.end(), rank) == dests.end()) {
        dests.push_back(rank);
      }
    }

   private:
    Buffer buffer;
    std::vector<int> dests;

    MPI_Comm comm;
    int tag;

    MPI_Request request;
    MPI_Status status;
  };

  using Ports = AssocVec<std::string, std::shared_ptr<Port>>;

 public:
  Component(Functor f, int rank, MPI_Comm comm = MPI_COMM_WORLD)
      : f(f), wanted(rank), comm(comm) {
    MPI_Comm_rank(comm, &actual);
  }

  const Buffer& get_in_port_buffer(std::string name) {
    return in_port.at(name)->get();
  }

  void make_in_port(std::string name) {
    inputs.try_emplace(name, Buffer());
    in_port.try_emplace(name, std::make_shared<Port>(comm));
  }

  const Buffer& get_out_port_buffer(std::string name) {
    return out_port.at(name)->get();
  }

  void make_out_port(std::string name) {
    outputs.try_emplace(name, Buffer());
    int tag = out_port.size();
    out_port.try_emplace(name, std::make_shared<Port>(comm, tag));
  }

  Buffer& get_input(std::string name) { return inputs.at(name); }
  Buffer& get_output(std::string name) { return outputs.at(name); }

  void connect(Component& target, std::string from, std::string to) {
    in_port.at(to) = target.out_port.at(from);
    target.out_port.at(from)->add_dest(wanted);
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
      if (wanted == actual) {
        port->set(outputs.index(i));
      }
      port->sync(wanted, actual);
    }
  }

 private:
  Functor f;
  int wanted;
  int actual;
  MPI_Comm comm;

  Dict inputs;
  Dict outputs;
  Ports in_port;
  Ports out_port;
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
