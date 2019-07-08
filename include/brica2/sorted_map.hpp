#ifndef __BRICA2_SORTED_MAP_HPP__
#define __BRICA2_SORTED_MAP_HPP__

#include "brica2/macros.h"

#include <utility>
#include <vector>

NAMESPACE_BEGIN(BRICA2_NAMESPACE)

template <
    class Key,
    class T,
    class Compare = std::less<Key>,
    class Allocator = std::allocator<std::pair<Key, T>>>
class sorted_map {
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

  template <class, class, class, class> friend class sorted_map;

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
  sorted_map() {}
  explicit sorted_map(const Compare& comp, const Allocator& alloc = Allocator())
      : data(vector_type(alloc)), compare(comp) {}

  explicit sorted_map(const Allocator& alloc) : data(vector_type(alloc)) {}

  template <class InputIt>
  sorted_map(
      InputIt first,
      InputIt last,
      const Compare& comp = Compare(),
      const Allocator& alloc = Allocator())
      : data(first, last, alloc), compare(comp) {}

  template <class InputIt>
  sorted_map(InputIt first, InputIt last, const Allocator& alloc)
      : data(first, last, alloc) {}

  sorted_map(const sorted_map& other) = default;
  sorted_map(const sorted_map& other, const Allocator& alloc)
      : data(other.data, alloc), compare(other.compare) {}

  sorted_map(sorted_map&& other) = default;
  sorted_map(sorted_map&& other, const Allocator& alloc)
      : data(other.data, alloc), compare(other.compare) {}

  sorted_map(
      std::initializer_list<value_type> init,
      const Compare& comp = Compare(),
      const Allocator& alloc = Allocator())
      : data(init, alloc), compare(comp) {}
  sorted_map(std::initializer_list<value_type> init, const Allocator& alloc)
      : data(init, alloc) {}

  ~sorted_map() {}

  sorted_map& operator=(const sorted_map& other) {
    return *this = sorted_map(other);
  }
  sorted_map& operator=(sorted_map&& other) = default;
  sorted_map& operator=(ilist_type ilist) { return *this = sorted_map(ilist); }

  allocator_type get_allocator() const { return data.get_allocator(); }

  /* Accessors */
  key_type& key(size_type pos) { return data.at(pos).first; }

  mapped_type& at(const key_type& key) {
    iterator lower = lower_bound(key);
    if (lower != end() && lower->first == key) {
      return lower->second;
    }
    throw std::out_of_range("sorted_map: " + key);
  }

  const mapped_type& at(const key_type& key) const {
    const_iterator lower = lower_bound(key);
    if (lower != cend() && lower->first == key) {
      return lower->second;
    }
    throw std::out_of_range("sorted_map: " + key);
  }

  mapped_type& index(size_type pos) { return data.at(pos).second; }
  const mapped_type& index(size_type pos) const { return data.at(pos).second; }

 private:
  template <bool> mapped_type& brace_impl(const key_type& key);
  template <bool> mapped_type& brace_impl(key_type&& key);

  template <> mapped_type& brace_impl<true>(const key_type& key) {
    return this->try_emplace(key).first->second;
  }

  template <> mapped_type& brace_impl<true>(key_type&& key) {
    return this->try_emplace(key).first->second;
  }

  template <> mapped_type& brace_impl<false>(const key_type& key) {
    return at(key);
  }

  template <> mapped_type& brace_impl<false>(key_type&& key) { return at(key); }

 public:
  mapped_type& operator[](const key_type& key) {
    return brace_impl<std::is_default_constructible<T>::value>(key);
  }

  mapped_type& operator[](key_type&& key) {
    return brace_impl<std::is_default_constructible<T>::value>(
        std::forward<key_type>(key));
  }

  const mapped_type& operator[](const key_type& key) const { return at(key); }
  const mapped_type& operator[](key_type&& key) const { return at(key); }

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
  template <class InputIt> void insert(InputIt first, InputIt last) {
    for (iterator it = first; it != last; ++it) {
      insert(it);
    }
  }

  void insert(ilist_type ilist) { insert(ilist.begin(), ilist.end()); }

  template <class... Args> std::pair<iterator, bool> emplace(Args&&... args) {
    return insert(value_type(std::forward<Args>(args)...));
  }

  template <class... Args>
  std::pair<iterator, bool> try_emplace(const key_type& key, Args&&... args) {
    iterator lower = lower_bound(key);
    if (lower != end() && lower->first == key) {
      return std::pair<iterator, bool>(lower, false);
    }
    return insert(value_type(
        std::piecewise_construct,
        std::forward_as_tuple(key),
        std::forward_as_tuple(std::forward<Args>(args)...)));
  }

  template <class... Args>
  std::pair<iterator, bool> try_emplace(key_type&& key, Args&&... args) {
    iterator lower = lower_bound(key);
    if (lower != end() && lower->first == key) {
      return std::pair<iterator, bool>(lower, false);
    }
    return insert(value_type(
        std::piecewise_construct,
        std::forward_as_tuple(std::move(key)),
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

  void swap(sorted_map& other) {
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
    const_iterator first = cbegin();
    const_iterator last = cend();
    using diff = typename std::iterator_traits<iterator>::difference_type;
    for (diff len = std::distance(first, last); len != 0;) {
      diff half = len / 2;
      const_iterator mid = first;
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
    const_iterator first = cbegin();
    const_iterator last = cend();
    using diff = typename std::iterator_traits<iterator>::difference_type;
    for (diff len = std::distance(first, last); len != 0;) {
      diff half = len / 2;
      const_iterator mid = first;
      if (!bool(compare(key, mid->first))) {
        len -= half + 1;
        first = ++mid;
      } else {
        len = half;
      }
    }
    return first;
  }

  /* Observers */
  key_compare key_comp() const { return compare; }
  value_compare value_comp() const { return value_compare(compare); }

  /* Non-member functions */
  friend bool operator==(const sorted_map& lhs, const sorted_map& rhs) {
    return lhs.data == rhs.data;
  }

  friend bool operator!=(const sorted_map& lhs, const sorted_map& rhs) {
    return lhs.data != rhs.data;
  }

  friend bool operator<=(const sorted_map& lhs, const sorted_map& rhs) {
    return lhs.data <= rhs.data;
  }

  friend bool operator<(const sorted_map& lhs, const sorted_map& rhs) {
    return lhs.data < rhs.data;
  }

  friend bool operator>=(const sorted_map& lhs, const sorted_map& rhs) {
    return lhs.data >= rhs.data;
  }

  friend bool operator>(const sorted_map& lhs, const sorted_map& rhs) {
    return lhs.data > rhs.data;
  }

  friend void swap(sorted_map& a, sorted_map& b) {
    using std::swap;
    swap(a.data, b.data);
    swap(a.compare, b.compare);
  }

 private:
  vector_type data;
  key_compare compare;
};

NAMESPACE_END(BRICA2_NAMESPACE)

#endif  // __BRICA2_SORTED_MAP_HPP__
