/******************************************************************************
 *
 * brica/buffer.hpp
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

#ifndef __BRICA_BUFFER_HPP__
#define __BRICA_BUFFER_HPP__

#include <memory>
#include <vector>

namespace brica {

class Buffer {
  using vector_type = std::vector<char>;
  using ilist_type = std::initializer_list<char>;

 public:
  using value_type = typename vector_type::value_type;
  using allocator_type = typename vector_type::allocator_type;
  using size_type = typename vector_type::size_type;
  using difference_type = typename vector_type::difference_type;
  using reference = typename vector_type::reference;
  using const_reference = typename vector_type::const_reference;
  using pointer = typename vector_type::pointer;
  using const_pointer = typename vector_type::const_pointer;
  using iterator = typename vector_type::iterator;
  using const_iterator = typename vector_type::const_iterator;
  using reverse_iterator = typename vector_type::reverse_iterator;
  using const_reverse_iterator = typename vector_type::const_reverse_iterator;

  /* Member functions */
  Buffer() : ptr(std::make_shared<vector_type>()) {}
  Buffer(size_type count, const value_type& value)
      : ptr(std::make_shared<vector_type>(count, value)) {}
  explicit Buffer(size_type count)
      : ptr(std::make_shared<vector_type>(count)) {}
  template <class InputIt>
  Buffer(InputIt first, InputIt last)
      : ptr(std::make_shared<vector_type>(first, last)) {}
  Buffer(const Buffer& other) : ptr(other.ptr) {}
  Buffer(Buffer&& other) : ptr(other.ptr) { other.ptr = nullptr; }
  Buffer(ilist_type init) : ptr(std::make_shared<vector_type>(init)) {}
  Buffer(vector_type v) : ptr(std::make_shared<vector_type>(v)) {}
  ~Buffer() {}

  Buffer& operator=(const Buffer& other) { return *this = Buffer(other); }
  Buffer& operator=(Buffer&& other) = default;
  Buffer& operator=(ilist_type ilist) { return *this = Buffer(ilist); }

  void assign(size_type count, const value_type& value) {
    ptr->assign(count, value);
  }

  template <class InputIterator>
  void assign(InputIterator first, InputIterator last) {
    ptr->assign(first, last);
  }

  void assign(ilist_type ilist) { ptr->assign(ilist); }

  allocator_type get_allocator() const { return ptr->get_allocator(); }

  /* Accessors */
  reference at(size_type pos) { return ptr->at(pos); }
  const_reference at(size_type pos) const { return ptr->at(pos); }
  reference operator[](size_type pos) { return (*ptr)[pos]; }
  const_reference operator[](size_type pos) const { return (*ptr)[pos]; }
  reference front() { return ptr->front(); }
  const_reference front() const { return ptr->front(); }
  reference back() { return ptr->back(); }
  const_reference back() const { return ptr->back(); }
  pointer data() noexcept { return ptr->data(); }
  const_pointer data() const noexcept { return ptr->data(); }

  /* Iterators */
  iterator begin() { return ptr->begin(); }
  const_iterator begin() const { return ptr->begin(); }
  const_iterator cbegin() const { return ptr->cbegin(); }

  iterator end() { return ptr->end(); }
  const_iterator end() const { return ptr->end(); }
  const_iterator cend() const { return ptr->cend(); }

  reverse_iterator rbegin() { return ptr->rbegin(); }
  const_reverse_iterator rbegin() const { return ptr->rbegin(); }
  const_reverse_iterator crbegin() const { return ptr->crbegin(); }

  reverse_iterator rend() { return ptr->rend(); }
  const_reverse_iterator rend() const { return ptr->rend(); }
  const_reverse_iterator crend() const { return ptr->crend(); }

  /* Capacity */
  bool empty() const noexcept { return ptr->empty(); }
  size_type size() const noexcept { return ptr->size(); }
  size_type max_size() const noexcept { return ptr->max_size(); }
  void reserve(size_type new_cap) { ptr->reserve(new_cap); }
  size_type capacity() const noexcept { return ptr->capacity(); }
  void shrink_to_fit() { ptr->shrink_to_fit(); }

  /* Modifiers */
  void clear() noexcept { ptr->clear(); }

  iterator insert(const_iterator pos, const value_type& value) {
    return ptr->insert(pos, value);
  }

  iterator insert(const_iterator pos, value_type&& value) {
    return ptr->insert(pos, std::forward<value_type>(value));
  }

  iterator insert(const_iterator pos, size_type count,
                  const value_type& value) {
    return ptr->insert(pos, count, value);
  }

  template <class InputIterator>
  iterator insert(iterator pos, InputIterator first, InputIterator last) {
    return ptr->insert(pos, first, last);
  }

  iterator insert(const_iterator pos, ilist_type ilist) {
    return ptr->insert(pos, ilist);
  }

  template <class... Args>
  iterator emplace(const_iterator pos, Args&&... args) {
    return ptr->emplace(pos, std::forward<Args>(args)...);
  }

  iterator erase(const_iterator pos) { return ptr->erase(pos); }
  iterator erase(const_iterator first, const_iterator last) {
    return ptr->erase(first, last);
  }

  void push_back(const value_type& value) { ptr->push_back(value); }
  void push_back(value_type&& value) {
    ptr->push_back(std::forward<value_type>(value));
  }

  template <class... Args>
  void emplace_back(Args&&... args) {
    ptr->emplace_back(std::forward<Args>(args)...);
  }

  void pop_back() { return ptr->pop_back(); }

  void resize(size_type count) { ptr->resize(count); }
  void resize(size_type count, const value_type& value) {
    ptr->resize(count, value);
  }

  void swap(Buffer& other) {
    using std::swap;
    swap(*this, other);
  }

  /* Non-member functions */
  friend bool operator==(const Buffer& lhs, const Buffer& rhs) {
    return *lhs.ptr == *rhs.ptr;
  }

  friend bool operator!=(const Buffer& lhs, const Buffer& rhs) {
    return *lhs.ptr != *rhs.ptr;
  }

  friend bool operator<=(const Buffer& lhs, const Buffer& rhs) {
    return *lhs.ptr >= *rhs.ptr;
  }

  friend bool operator<(const Buffer& lhs, const Buffer& rhs) {
    return *lhs.ptr < *rhs.ptr;
  }

  friend bool operator>=(const Buffer& lhs, const Buffer& rhs) {
    return *lhs.ptr >= *rhs.ptr;
  }

  friend bool operator>(const Buffer& lhs, const Buffer& rhs) {
    return *lhs.ptr > *rhs.ptr;
  }

  friend void swap(Buffer& a, Buffer& b) {
    using std::swap;
    swap(a.ptr, b.ptr);
  }

 private:
  std::shared_ptr<vector_type> ptr;
};

}  // namespace brica

#endif  // __BRICA_BUFFER_HPP__
