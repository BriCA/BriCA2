#ifndef __BRICA2_MPI_COMPONENT_HPP__
#define __BRICA2_MPI_COMPONENT_HPP__

#include "brica2/component.hpp"
#include "brica2/logger.hpp"
#include "brica2/mpi/datatype.hpp"

#include <exception>
#include <initializer_list>
#include <memory>

#include <string>
#include <sstream>

#include "mpi.h"

namespace brica2 {
namespace mpi {
namespace detail {

std::string format_error(std::string method, int errorcode) {
  std::stringstream ss;
  int rank, errorclass, resultlen;
  char errorstring[MPI_MAX_ERROR_STRING];
  MPI_Error_class(errorcode, &errorclass);
  MPI_Error_string(errorcode, errorstring, &resultlen);
  ss << method << " at rank " << rank << " error with code " << errorclass
     << ": " << std::string(errorstring, resultlen);
  std::string ret = ss.str();
  return ret;
}

}  // namespace detail

class mpi_exception : public std::exception {
 public:
  mpi_exception(std::string method, int errorcode)
      : method_(method), errorcode_(errorcode) {}

  const char* what() const noexcept override {
    return detail::format_error(method_, errorcode_).c_str();
  }

 private:
  std::string method_;
  int errorcode_;
};

void handle_error(int errorcode) {
  if (errorcode == MPI_SUCCESS) return;
  brica2::logger::error(mpi_exception("MPI_Isend", errorcode));
}

class bad_rank : public std::exception {
 public:
  const char* what() const noexcept override {
    return "called from invalid rank";
  }
};

class component : public component_type {
 public:
  explicit component(
      const functor_type& f, int rank, MPI_Comm comm = MPI_COMM_WORLD)
      : base(f), wanted_rank(rank) {
    MPI_Comm_rank(comm, &actual_rank);
  }

  explicit component(functor_type&& f, int rank, MPI_Comm comm = MPI_COMM_WORLD)
      : base(std::forward<functor_type>(f)), wanted_rank(rank) {
    MPI_Comm_rank(comm, &actual_rank);
  }

  component(const component&) = default;
  component(component&&) = default;
  component& operator=(const component&) = default;
  component& operator=(component&&) = default;

  virtual ~component() {}

  bool enabled() const { return wanted_rank == actual_rank; }

  template <class T, class S = std::initializer_list<ssize_t>>
  void make_in_port(const std::string& key, S&& s) {
    if (enabled()) base.make_in_port<T>(key, std::forward<S>(s));
  }

  template <class T, class S = std::initializer_list<ssize_t>>
  void make_out_port(const std::string& key, S&& s) {
    if (enabled()) base.make_out_port<T>(key, std::forward<S>(s));
  }

  port& get_in_port(const std::string& key) {
    if (enabled()) return base.get_in_port(key);
    throw bad_rank();
  }

  port& get_out_port(const std::string& key) {
    if (enabled()) return base.get_out_port(key);
    throw bad_rank();
  }

  buffer& get_input(const std::string& key) {
    if (enabled()) return get_input(key);
    throw bad_rank();
  }

  buffer& get_output(const std::string& key) {
    if (enabled()) return get_output(key);
    throw bad_rank();
  }

  virtual void collect() override {
    if (enabled()) base.collect();
  }

  virtual void execute() override {
    if (enabled()) base.execute();
  }

  virtual void expose() override {
    if (enabled()) base.expose();
  }

 private:
  basic_component base;
  int wanted_rank;
  int actual_rank;
};

struct singular_io {
  virtual bool sending() const = 0;
  virtual bool receiving() const = 0;
  virtual port& get_in_port() = 0;
  virtual port& get_out_port() = 0;
};

template <class T> class proxy : public component_type, public singular_io {
 public:
  template <class S = std::initializer_list<ssize_t>>
  proxy(S&& s, int src, int dest, int tag = 0, MPI_Comm comm = MPI_COMM_WORLD)
      : src(src), dest(dest), tag(tag), comm(comm), request(MPI_REQUEST_NULL) {
    MPI_Comm_rank(comm, &rank);
    if (rank == src) in_port.reshape<T>(std::forward<S>(s));
    if (rank == dest) out_port.reshape<T>(std::forward<S>(s));
    if (rank == src || rank == dest) {
      memory = std::make_shared<buffer>(std::forward<S>(s), T());
    }
  }

  virtual bool sending() const override { return rank == src; }
  virtual bool receiving() const override { return rank == dest; }

  virtual port& get_in_port() override {
    if (sending()) return in_port;
    throw bad_rank();
  }

  virtual port& get_out_port() override {
    if (receiving()) return out_port;
    throw bad_rank();
  }

  void send() {
    void* buf = memory->data();
    int count = memory->size();
    handle_error(
        MPI_Isend(buf, count, datatype<T>(), dest, tag, comm, &request));
  }

  void recv() {
    void* buf = memory->data();
    int count = memory->size();
    handle_error(
        MPI_Irecv(buf, count, datatype<T>(), src, tag, comm, &request));
  }

  virtual void collect() override {
    if (sending()) *memory = in_port.get();
  }

  virtual void execute() override {
    if (sending()) send();
    if (receiving()) recv();
  }

  virtual void expose() override {
    if (request != MPI_REQUEST_NULL) handle_error(MPI_Wait(&request, &status));
    request = MPI_REQUEST_NULL;
    if (receiving()) out_port.set(*memory);
  }

 private:
  int src;
  int dest;
  int tag;
  MPI_Comm comm;

  port in_port;
  port out_port;
  std::shared_ptr<buffer> memory;

  int rank;
  MPI_Status status;
  MPI_Request request;
};

template <class T, class S = std::initializer_list<ssize_t>>
class broadcast : public component_type, public singular_io {
 public:
  broadcast(S&& s, int root, MPI_Comm comm = MPI_COMM_WORLD)
      : root(root), comm(comm), memory(empty(std::forward<S>(s), T())) {
    MPI_Comm_rank(comm, &rank);
  }

  virtual bool sending() const override { return rank == root; }
  virtual bool receiving() const override { return rank != root; }

  virtual port& get_in_port() override {
    if (sending()) return in_port;
  }

  virtual port& get_out_port() override {
    if (receiving()) return out_port;
  }

  virtual void collect() override {
    if (sending()) *memory = in_port.get();
  }

  virtual void execute() override {
    void* buf = memory->data();
    int count = memory->size();
    handle_error(MPI_Bcast(buf, count, datatype<T>(), root, comm));
  }

  virtual void expose() override {
    if (receiving()) out_port.set(*memory);
  }

 private:
  int root;
  MPI_Comm comm;

  port in_port;
  port out_port;
  std::shared_ptr<buffer> memory;

  int rank;
};

struct port_spec {
  component& c;
  std::string k;
};

inline void connect(port_spec&& source, port_spec&& target) {
  if (!source.c.enabled() || !target.c.enabled()) return;

  auto& source_port = source.c.get_out_port(source.k);
  auto& target_port = target.c.get_in_port(target.k);

  if (!compatible(source_port.get(), target_port.get())) {
    throw incompatible_exception();
  }

  target_port = source_port;
}

inline void connect(port_spec&& source, singular_io& target) {
  if (!source.c.enabled() || !target.sending()) return;

  auto& source_port = source.c.get_out_port(source.k);
  auto& target_port = target.get_in_port();

  if (!compatible(source_port.get(), target_port.get())) {
    throw incompatible_exception();
  }

  target_port = source_port;
}

inline void connect(singular_io& source, port_spec&& target) {
  if (!source.receiving() || !target.c.enabled()) return;

  auto& source_port = source.get_out_port();
  auto& target_port = target.c.get_in_port(target.k);

  if (!compatible(source_port.get(), target_port.get())) {
    throw incompatible_exception();
  }

  target_port = source_port;
}

}  // namespace mpi
}  // namespace brica2

#endif  // __BRICA2_MPI_COMPONENT_HPP__
