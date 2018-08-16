#ifndef __BRICA_PORT_HPP__
#define __BRICA_PORT_HPP__

#include "brica/assocvec.hpp"

#include <memory>

namespace brica {

template <class T>
class Port {
 public:
  void set(T& value) { buffer = value; }
  const T& get() const { return buffer; }

 private:
  T buffer;
};

template <class T>
using Ports = AssocVec<std::string, std::shared_ptr<Port<T>>>;

}  // namespace brica

#endif  // __BRICA_PORT_HPP__
