/******************************************************************************
 *
 * brica/port.hpp
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

#ifndef __BRICA_PORT_HPP__
#define __BRICA_PORT_HPP__

#include "brica/assocvec.hpp"

#include <memory>

namespace brica {

template <class T>
class Port {
 public:
  Port(T init = T()) : buffer(init) {}

  void set(T& value) { buffer = value; }
  T& get() { return buffer; }

 private:
  T buffer;
};

template <class T>
using Ports = AssocVec<std::string, std::shared_ptr<Port<T>>>;

}  // namespace brica

#endif  // __BRICA_PORT_HPP__
