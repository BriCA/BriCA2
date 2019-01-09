/******************************************************************************
 *
 * brica/assocvec.hpp
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

#ifndef __BRICA_ASSOCVEC_HPP__
#define __BRICA_ASSOCVEC_HPP__

#include <algorithm>
#include <functional>
#include <memory>
#include <vector>

namespace brica {

template <class Key, class T, class Compare = std::less<Key>,
          class Allocator = std::allocator<std::pair<Key, T>>>
class AssocVec {
 public:
  using key_type = Key;
  using mapped_type = T;
  using value_type = std::pair<key_type, mapped_type>;
  using difference_type = std::ptrdiff_t;
  using key_compare = Compare;
  using allocator_type = Allocator;

 private:
  using allocator_traits_type = std::allocator_traits<allocator_type>;
  using vector_type = std::vector<value_type, allocator_type>;
  using ilist_type = std::initializer_list<value_type>;

 public:
  using size_type = typename vector_type::size_type;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = typename allocator_traits_type::pointer;
  using const_pointer = typename allocator_traits_type::const_pointer;
  using iterator = typename vector_type::iterator;
  using const_iterator = typename vector_type::const_iterator;
  using reverse_iterator = typename vector_type::reverse_iterator;
  using const_reverse_iterator = typename vector_type::const_reverse_iterator;

  class value_compare {
   protected:
    value_compare(Compare comp) : compare(comp) {}

   public:
    bool operator()(const value_type& lhs, const value_type& rhs) const {
      return compare(lhs.first, rhs.first);
    }

   private:
    Compare compare;
  };

 private:
  static bool compare_key(const value_type& lhs, const value_type& rhs) {
    return lhs.first < rhs.first;
  }

 public:
  /* Member functions */
  AssocVec() {}
  explicit AssocVec(const Compare& comp, const Allocator& alloc = Allocator())
      : data(vector_type(alloc)), compare(comp) {}

  explicit AssocVec(const Allocator& alloc) : data(vector_type(alloc)) {}

  template <class InputIterator>
  AssocVec(InputIterator first, InputIterator last,
           const Compare& comp = Compare(),
           const Allocator& alloc = Allocator())
      : data(first, last, alloc), compare(comp) {}

  template <class InputIterator>
  AssocVec(InputIterator first, InputIterator last, const Allocator& alloc)
      : data(first, last, alloc) {}

  AssocVec(const AssocVec& other) = default;
  AssocVec(const AssocVec& other, const Allocator& alloc)
      : data(other.data, alloc), compare(other.compare) {}

  AssocVec(AssocVec&& other) = default;
  AssocVec(AssocVec&& other, const Allocator& alloc)
      : data(other.data, alloc), compare(other.compare) {}

  AssocVec(std::initializer_list<value_type> init,
           const Compare& comp = Compare(),
           const Allocator& alloc = Allocator())
      : data(init, alloc), compare(comp) {}
  AssocVec(std::initializer_list<value_type> init, const Allocator& alloc)
      : data(init, alloc) {}

  ~AssocVec() {}

  AssocVec& operator=(const AssocVec& other) { return *this = AssocVec(other); }
  AssocVec& operator=(AssocVec&& other) = default;
  AssocVec& operator=(ilist_type ilist) { return *this = AssocVec(ilist); }

  allocator_type get_allocator() const { return data.get_allocator(); }

  /* Accessors */
  key_type& key(size_type pos) { return data.at(pos).first; }

  mapped_type& at(const key_type& key) {
    iterator lower = lower_bound(key);
    if (lower != end() && lower->first == key) {
      return lower->second;
    }
    throw std::out_of_range("AssocVec: " + key);
  }

  const mapped_type& at(const key_type& key) const { return at(key); }

  mapped_type& index(size_type pos) { return data.at(pos).second; }
  const mapped_type& index(size_type pos) const { return data.at(pos).second; }

  mapped_type& operator[](const key_type& key) {
    return this->try_emplace(key).first->second;
  }

  mapped_type& operator[](key_type&& key) {
    return this->try_emplace(std::forward<key_type>(key)).first->second;
  }

  const mapped_type& operator[](const key_type& key) const { return at(key); }

  /* Iterators */
  iterator begin() noexcept { return data.begin(); }
  const_iterator begin() const noexcept { return data.begin(); }
  const_iterator cbegin() const noexcept { return data.cbegin(); }

  iterator end() noexcept { return data.end(); }
  const_iterator end() const noexcept { return data.end(); }
  const_iterator cend() const noexcept { return data.cend(); }

  iterator rbegin() noexcept { return data.rbegin(); }
  const_iterator rbegin() const noexcept { return data.rbegin(); }
  const_iterator crbegin() const noexcept { return data.crbegin(); }

  iterator rend() noexcept { return data.rend(); }
  const_iterator rend() const noexcept { return data.rend(); }
  const_iterator crend() const noexcept { return data.crend(); }

  /* Capacity */
  bool empty() const noexcept { return data.empty(); }
  size_type size() const noexcept { return data.size(); }
  size_type max_size() const noexcept { return data.max_size(); }
  void reserve(size_type new_cap) { data.reserve(new_cap); }
  size_type capacity() const noexcept { return data.capacity(); }
  void shrink_to_fit() { data.shrink_to_fit(); }

  /* Modifiers */
  void clear() noexcept { data.clear(); }

  std::pair<iterator, bool> insert(const value_type& value) {
    iterator lower = lower_bound(value.first);
    if (lower != end() && *lower == value) {
      return std::pair<iterator, bool>(lower, false);
    }
    return std::pair<iterator, bool>(data.insert(lower, value), true);
  }

  std::pair<iterator, bool> insert(value_type&& value) { return insert(value); }
  template <class InputIterator>
  void insert(InputIterator first, InputIterator last) {
    for (iterator it = first; it != last; ++it) {
      insert(it);
    }
  }

  void insert(ilist_type ilist) { insert(ilist.begin(), ilist.end()); }

  template <class... Args>
  std::pair<iterator, bool> emplace(Args&&... args) {
    return insert(value_type(std::forward<Args>(args)...));
  }

  template <class... Args>
  std::pair<iterator, bool> try_emplace(const key_type& key, Args&&... args) {
    iterator lower = lower_bound(key);
    if (lower != end() && lower->first == key) {
      return std::pair<iterator, bool>(lower, false);
    }
    return insert(
        value_type(std::piecewise_construct, std::forward_as_tuple(key),
                   std::forward_as_tuple(std::forward<Args>(args)...)));
  }

  template <class... Args>
  std::pair<iterator, bool> try_emplace(key_type&& key, Args&&... args) {
    iterator lower = lower_bound(key);
    if (lower != end() && lower->first == key) {
      return std::pair<iterator, bool>(lower, false);
    }
    return insert(value_type(
        std::piecewise_construct, std::forward_as_tuple(std::move(key)),
        std::forward_as_tuple(std::forward<Args>(args)...)));
  }

  iterator erase(const_iterator pos) { return data.erase(pos); }

  iterator erase(const_iterator first, const_iterator last) {
    return data.erase(first, last);
  }

  size_type erase(const key_type& key) {
    iterator lower = lower_bound(key);
    if (lower != end() && lower->first == key) {
      erase(lower);
      return 1;
    }
    return 0;
  }

  void swap(AssocVec& other) {
    using std::swap;
    swap(*this, other);
  }

  /* Lookup */
  size_type count(const key_type& key) const { return contains(key) ? 1 : 0; }

  iterator find(const key_type& key) {
    iterator last = end();
    iterator lower = lower_bound(key);
    if (lower != last && lower->first == key) {
      return lower;
    }
    return last;
  }

  const_iterator find(const key_type& key) const { return find(key); }

  bool contains(const key_type& key) const {
    return binary_search(begin(), end(), key, compare_key);
  }

  std::pair<iterator, iterator> equal_range(const key_type& key) {
    return std::equal_range(begin(), end(), key, compare_key);
  }

  std::pair<const_iterator, const_iterator> equal_range(
      const key_type& key) const {
    return std::equal_range(begin(), end(), key, compare_key);
  }

  iterator lower_bound(const key_type& key) {
    iterator first = begin();
    iterator last = end();
    using diff = typename std::iterator_traits<iterator>::difference_type;
    for (diff len = std::distance(first, last); len != 0;) {
      diff half = len / 2;
      iterator mid = first;
      std::advance(mid, half);
      if (compare(mid->first, key)) {
        len -= half + 1;
        first = ++mid;
      } else {
        len = half;
      }
    }
    return first;
  }

  const_iterator lower_bound(const key_type& key) const {
    return lower_bound(key);
  }

  iterator upper_bound(const key_type& key) {
    iterator first = begin();
    iterator last = end();
    using diff = typename std::iterator_traits<iterator>::difference_type;
    for (diff len = std::distance(first, last); len != 0;) {
      diff half = len / 2;
      iterator mid = first;
      if (!bool(compare(key, mid->first))) {
        len -= half + 1;
        first = ++mid;
      } else {
        len = half;
      }
    }
    return first;
  }

  const_iterator upper_bound(const key_type& key) const {
    return upper_bound(key);
  }

  /* Observers */
  key_compare key_comp() const { return compare; }
  value_compare value_comp() const { return value_compare(compare); }

  /* Non-member functions */
  friend bool operator==(const AssocVec& lhs, const AssocVec& rhs) {
    return lhs.data == rhs.data;
  }

  friend bool operator!=(const AssocVec& lhs, const AssocVec& rhs) {
    return lhs.data != rhs.data;
  }

  friend bool operator<=(const AssocVec& lhs, const AssocVec& rhs) {
    return lhs.data <= rhs.data;
  }

  friend bool operator<(const AssocVec& lhs, const AssocVec& rhs) {
    return lhs.data < rhs.data;
  }

  friend bool operator>=(const AssocVec& lhs, const AssocVec& rhs) {
    return lhs.data >= rhs.data;
  }

  friend bool operator>(const AssocVec& lhs, const AssocVec& rhs) {
    return lhs.data > rhs.data;
  }

  friend void swap(AssocVec& a, AssocVec& b) {
    using std::swap;
    swap(a.data, b.data);
    swap(a.compare, b.compare);
  }

 private:
  vector_type data;
  key_compare compare;
};

}  // namespace brica

#endif  // __BRICA_ASSOCVEC_HPP__
