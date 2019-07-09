#ifndef __BRICA2_MPI_COMPONENT_HPP__
#define __BRICA2_MPI_COMPONENT_HPP__

#include "brica2/component.hpp"
#include "brica2/mpi/datatype.hpp"

#include <initializer_list>

#include "mpi.h"

NAMESPACE_BEGIN(BRICA2_NAMESPACE)
NAMESPACE_BEGIN(mpi)

class component : public basic_component {
 public:
  explicit component(
      const functor_type& f, int rank, MPI_Comm comm = MPI_COMM_WORLD)
      : basic_component(f), wanted_rank(rank) {}

  explicit component(functor_type&& f, int rank, MPI_Comm comm = MPI_COMM_WORLD)
      : basic_component(std::forward<functor_type>(f)), wanted_rank(rank) {
    MPI_Comm_rank(comm, &actual_rank);
  }

  virtual ~component() {}

  virtual void collect() override {
    if (wanted_rank == actual_rank) basic_component::collect();
  }

  virtual void execute() override {
    if (wanted_rank == actual_rank) basic_component::execute();
  }

  virtual void expose() override {
    if (wanted_rank == actual_rank) basic_component::expose();
  }

 private:
  int wanted_rank;
  int actual_rank;
};

struct singular_io {
  virtual port& get_in_port() = 0;
  virtual port& get_out_port() = 0;
};

template <class T> class proxy : public component_type, public singular_io {
 public:
  template <class S = std::initializer_list<ssize_t>>
  proxy(S&& s, int src, int dest, int tag = 0, MPI_Comm comm = MPI_COMM_WORLD)
      : src(src),
        dest(dest),
        tag(tag),
        comm(comm),
        in_port(std::forward<S>(s), T()),
        out_port(std::forward<S>(s), T()),
        memory(fill(std::forward<S>(s), T())) {
    MPI_Comm_rank(comm, &rank);
  }

  virtual port& get_in_port() override { return in_port; }
  virtual port& get_out_port() override { return out_port; }

  void send() {
    void* buf = memory.data();
    int count = memory.size();
    MPI_Isend(buf, count, datatype<T>(), dest, tag, comm, &request);
  }

  void recv() {
    void* buf = memory.data();
    int count = memory.size();
    MPI_Irecv(buf, count, datatype<T>(), src, tag, comm, &request);
  }

  virtual void collect() override {
    if (rank == src) memory = in_port.get();
  }

  virtual void execute() override {
    if (rank == src) send();
    if (rank == dest) recv();
  }

  virtual void expose() override {
    if (rank == src || rank == dest) MPI_Wait(&request, &status);
    if (rank == dest) out_port.set(memory);
  }

 private:
  int src;
  int dest;
  int tag;
  MPI_Comm comm;
  port in_port;
  port out_port;
  buffer memory;

  int rank;
  MPI_Status status;
  MPI_Request request;
};

template <class T, class S = std::initializer_list<ssize_t>>
class broadcast : public component_type, public singular_io {
 public:
  broadcast(S&& s, int root, MPI_Comm comm = MPI_COMM_WORLD)
      : root(root),
        comm(comm),
        in_port(std::forward<S>(s), T()),
        out_port(std::forward<S>(s), T()),
        memory(fill(std::forward<S>(s), T())) {
    MPI_Comm_rank(comm, &rank);
  }

  virtual port& get_in_port() override { return in_port; }
  virtual port& get_out_port() override { return out_port; }

  virtual void collect() override {
    if (rank == root) memory = in_port.get();
  }

  virtual void execute() override {
    void* buf = memory.data();
    int count = memory.size();
    MPI_Bcast(buf, count, datatype<T>(), root, comm);
  }

  virtual void expose() override {
    if (rank != root) out_port.set(memory);
  }

 private:
  int root;
  MPI_Comm comm;
  port in_port;
  port out_port;
  buffer memory;

  int rank;
};

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

inline void connect(port_spec&& source, singular_io& target) {
  auto& source_port = source.c.get_out_port(source.k);
  auto& target_port = target.get_in_port();
  if (!compatible(source_port.get(), target_port.get())) {
    throw incompatible_exception();
  }
  target_port = source_port;
}

inline void connect(singular_io& source, port_spec&& target) {
  auto& source_port = source.get_out_port();
  auto& target_port = target.c.get_in_port(target.k);
  if (!compatible(source_port.get(), target_port.get())) {
    throw incompatible_exception();
  }
  target_port = source_port;
}

NAMESPACE_END(mpi)
NAMESPACE_END(BRICA2_NAMESPACE)

#endif  // __BRICA2_MPI_COMPONENT_HPP__
