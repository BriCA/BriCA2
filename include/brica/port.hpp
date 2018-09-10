#ifndef __BRICA_PORT_HPP__
#define __BRICA_PORT_HPP__

#include "brica/assocvec.hpp"

#include <memory>

namespace brica {

template <class T>
class Port {
 public:
  void set(T& value) { buffer = value; }
  T& get() { return buffer; }

 private:
  T buffer;
};

template <typename T>
using pPort = std::shared_ptr<Port<T>>;

template <class T>
using Ports = AssocVec<std::string, pPort<T>>;

}  // namespace brica

#endif  // __BRICA_PORT_HPP__
