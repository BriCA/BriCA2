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

class Channel {
 public:
  Channel(int src, int dest, int tag) : src(src), dest(dest), tag(tag) {
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  }

  void sync_size(int size) {
    if ((rank == src && rank == dest) || (rank != src && rank != dest)) return;
    _size = size;
    if (rank == src) {
      MPI_Isend(&_size, 1, MPI_INT, dest, tag, MPI_COMM_WORLD, &request);
    }
    if (rank == dest) {
      MPI_Irecv(&_size, 1, MPI_INT, src, tag, MPI_COMM_WORLD, &request);
    }
  }

  void sync_buffer(Buffer buffer) {
    if ((rank == src && rank == dest) || (rank != src && rank != dest)) return;
    if (rank == src) {
      char* data = buffer.data();
      MPI_Isend(data, _size, MPI_CHAR, dest, tag, MPI_COMM_WORLD, &request);
    }
    if (rank == dest) {
      buffer.resize(_size);
      char* data = buffer.data();
      MPI_Irecv(data, _size, MPI_CHAR, src, tag, MPI_COMM_WORLD, &request);
    }
  }

  void wait() {
    if ((rank == src && rank == dest) || (rank != src && rank != dest)) return;
    MPI_Wait(&request, &status);
  }

 private:
  int rank;
  int src;
  int dest;
  int tag;

  MPI_Request request;
  MPI_Status status;

  int _size;
};

class Component {
  struct Port {
    Port(int src, int tag) : src(src), tag(tag) {}

    void sync_size() {
      for (std::size_t i = 0; i < channels.size(); ++i) {
        channels[i].sync_size(buffer.size());
      }
    }

    void sync_buffer() {
      for (std::size_t i = 0; i < channels.size(); ++i) {
        channels[i].sync_buffer(buffer);
      }
    }

    void join() {
      for (std::size_t i = 0; i < channels.size(); ++i) {
        channels[i].wait();
      }
    }

    void set(Buffer& value) { buffer = value; }
    const Buffer& get() const { return buffer; }

    void add_dest(int dest) {
      if (std::find(dests.begin(), dests.end(), dest) == dests.end()) {
        dests.push_back(dest);
        channels.push_back(Channel(src, dest, tag));
      }
    }

   private:
    int src;
    int tag;

    Buffer buffer;
    std::vector<int> dests;
    std::vector<Channel> channels;
  };

  using Ports = AssocVec<std::string, std::shared_ptr<Port>>;

 public:
  Component(Functor f, int rank) : f(f), wanted(rank) {
    MPI_Comm_rank(MPI_COMM_WORLD, &actual);
  }

  const Buffer& get_in_port_buffer(std::string name) {
    return in_port.at(name)->get();
  }

  void make_in_port(std::string name) {
    inputs.try_emplace(name, Buffer());
    in_port.try_emplace(name, std::make_shared<Port>(wanted, 0));
  }

  const Buffer& get_out_port_buffer(std::string name) {
    return out_port.at(name)->get();
  }

  void make_out_port(std::string name) {
    outputs.try_emplace(name, Buffer());
    out_port.try_emplace(name, std::make_shared<Port>(wanted, out_port.size()));
  }

  Buffer& get_input(std::string name) { return inputs.at(name); }
  Buffer& get_output(std::string name) { return outputs.at(name); }

  void connect(Component& target, std::string from, std::string to) {
    in_port[to] = target.out_port[from];
    target.out_port[from]->add_dest(wanted);
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

  void pre_expose() {
    for (std::size_t i = 0; i < outputs.size(); ++i) {
      auto port = out_port.index(i);
      if (wanted == actual) {
        port->set(outputs.index(i));
      }
      port->sync_size();
    }
  }

  void post_expose() {
    for (std::size_t i = 0; i < outputs.size(); ++i) {
      out_port.index(i)->sync_buffer();
    }
  }

  void join() {
    for (std::size_t i = 0; i < outputs.size(); ++i) {
      out_port.index(i)->join();
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

class VirtualTimeSyncScheduler {
 public:
  void add_component(Component* component) { components.push_back(component); }

  void step() {
    for (std::size_t i = 0; i < components.size(); ++i) {
      components[i]->collect();
      components[i]->execute();
    }

    for (std::size_t i = 0; i < components.size(); ++i) {
      components[i]->pre_expose();
    }

    for (std::size_t i = 0; i < components.size(); ++i) {
      components[i]->join();
    }

    for (std::size_t i = 0; i < components.size(); ++i) {
      components[i]->post_expose();
    }

    for (std::size_t i = 0; i < components.size(); ++i) {
      components[i]->join();
    }

    MPI_Barrier(MPI_COMM_WORLD);
  }

 private:
  std::vector<Component*> components;
};

class VirtualTimeScheduler {
  struct Event {
    Time time;
    Component* component;
    Timing timing;
    bool sleep;

    bool operator<(const Event& rhs) const { return time > rhs.time; }
  };

 public:
  void add_component(Component* component, Timing timing) {
    event_queue.push({timing.offset, component, timing, false});
  }

  void step() {
    Time time = event_queue.top().time;

    std::queue<Component*> awake;
    std::queue<Component*> asleep;

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

    std::queue<Component*> tmp;

    for (tmp = asleep; !tmp.empty(); tmp.pop()) {
      tmp.front()->pre_expose();
    }

    for (tmp = asleep; !tmp.empty(); tmp.pop()) {
      tmp.front()->join();
    }

    for (tmp = asleep; !tmp.empty(); tmp.pop()) {
      tmp.front()->post_expose();
    }

    for (tmp = asleep; !tmp.empty(); tmp.pop()) {
      tmp.front()->join();
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
